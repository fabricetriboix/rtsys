#!/usr/bin/env python

# Copyright (c) 2014-2016  Fabrice Triboix
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import sys
import argparse
import os


class MapBuilder:
    """
    Build a map of test groups from unit test source files

    The `Build()` method returns a dictionary of groups indexed by group ids.

    Each group is itself a dictionary with the following elements:
     - 'name': Group name
     - 'gid': Group unique identifier (as integer)
     - 'path': Path of the corresponding unit test source file
     - 'line': Line in the above source file that defines this group
     - 'cases': A list of test cases (see below)

    Each test case is a dictionary that looks like this:
     - 'name': Test case name
     - 'path': Path of the unit test source file that defines this test case
     - 'line': Line in the above source file where this test case is defined
    """

    rtt_group_start = "RTT_GROUP_START"
    rtt_group_end = "RTT_GROUP_END"
    rtt_test_start = "RTT_TEST_START"
    rtt_test_end = "RTT_TEST_END"

    def Build(self, srcfiles):
        self.groups = {}
        for path in srcfiles:
            with  open(path, 'r') as f:
                self.parse(f, path)
        return self.groups

    def Dump(self, rttmap):
        for group in rttmap.values():
            print("---")
            print("name: {}".format(group['name']))
            print("gid : 0x{:08x}".format(group['gid']))
            print("path: {}".format(group['path']))
            print("line: {}".format(group['line']))
            print("test cases:")
            for tc in group['cases']:
                print("  {} - {}:{}".format(tc['name'], tc['path'], tc['line']))

    def splitLine(self, line):
        tokens = line.strip().split("(")
        if len(tokens) != 2:
            return []
        ret = [tokens[0].strip()]
        tokens = tokens[1].strip().split(")")
        if len(tokens) < 1:
            return []
        tokens = tokens[0].strip().split(",")
        for t in tokens:
            ret.append(t.strip())
        return ret

    def parse(self, f, path):
        inGroup = False
        lineno = 0
        group = {}
        for line in f:
            lineno += 1
            tokens = self.splitLine(line)

            if len(tokens) < 1:
                pass

            elif tokens[0] == self.rtt_group_start:
                if inGroup:
                    raise RuntimeError("Invalid source file {}:{} - "
                        "group starts before previous group ends"
                        .format(path, lineno))
                tmp = tokens[2]
                if tmp[-1:].lower() == "u":
                    tmp = tmp[:-1]
                gid = 0
                try:
                    gid = int(tmp, 0)
                except:
                    raise RuntimeError("Invalid source file {}:{} - "
                        "invalid group id {}".format(path, lineno, tokens[2]))
                if gid in self.groups:
                    raise RuntimeError("Invalid source file {}:{} - "
                        "group id {} already used by group {} (file {}:{})"
                        .format(path, lineno, tokens[2],
                            self.groups[gid]['name'],
                            self.groups[gid]['path'],
                            self.groups[gid]['line']))
                inGroup = True
                group = { 'name': tokens[1], 'gid': gid,
                    'path': path, 'line': lineno, 'cases': [] }

            elif tokens[0] == self.rtt_test_start:
                if not inGroup:
                    raise RuntimeError("Invalid source file {}:{} - "
                        "test case outside a group"
                        .format(path, lineno))
                case = { 'name': tokens[1], 'path': path, 'line': lineno }
                group['cases'].append(case)

            elif line.strip().startswith(self.rtt_group_end):
                self.groups[group['gid']] = group
                inGroup = False

            else:
                pass # Nothing to do with other lines


class RttParser:
    """A class to parse an rttest output file and print its content as
    plain text"""

    rtt_format_fourcc = "RTTa"
    rtt_format_enter_group = ">"
    rtt_format_exit_group = "<"
    rtt_format_end_of_file = "*"
    failed = 0
    total = 0

    def Parse(self, rttpath, rttmap, verbose):
        self.failed = 0
        self.total = 0
        f = sys.stdin
        if rttpath != "-":
            f = open(rttpath, 'rb')
        data = f.read(len(self.rtt_format_fourcc))
        if data != self.rtt_format_fourcc:
            raise RuntimeError("Invalid rttest file {}: Invalid fourcc"
                .format(rttpath))
        eof = False
        while not eof:
            eof = self.parseGroup(f, rttpath, rttmap, verbose)
        if self.failed > 0:
            print("UNIT TESTS FAIL - {}/{} unit tests failed"
                .format(self.failed, self.total))
        else:
            print("UNIT TEST PASS - {}/{} unit tests passed"
                .format(self.total, self.total))
        return self.failed <= 0

    def parseGroup(self, f, rttpath, rttmap, verbose):
        # Parse group start character, or end-of-file character
        byte = self.readByte(f, rttpath)
        if byte == self.rtt_format_end_of_file:
            return True
        if byte != self.rtt_format_enter_group:
            raise RuntimeError("Invalid rttest file {}: Expected group start "
                "marker at byte {}".format(rttpath, f.tell() - 1))

        # Parse group id (big-endian format)
        gid = 0
        for i in range(0, 4):
            byte = self.readByte(f, rttpath)
            gid = (gid << 8) | (ord(byte) & 0xff)
        if not gid in rttmap:
            raise RuntimeError("Invalid rttest file {}: Unknown group id "
                "{:08x}, at byte {} - Did you provide all the source files "
                "on the command line?".format(rttpath, gid, f.tell() - 1))
        group = rttmap[gid]
        if verbose:
            print("{} 0x{:08x}".format(group['name'], gid))

        # Parse test case results
        results = 0
        testno = 0
        bits = 0
        for tc in group['cases']:
            if (testno % 8) == 0:
                byte = self.readByte(f, rttpath)
                bits = ord(byte)
            if bits & 1:
                self.failed += 1
                print("  FAIL {} at {}:{}"
                    .format(tc['name'], tc['path'], tc['line']))
            elif verbose:
                print("  PASS {}".format(tc['name']))
            bits = bits >> 1
            testno += 1
            self.total += 1

        # Parse end of group character
        byte = self.readByte(f, rttpath)
        if byte != self.rtt_format_exit_group:
            raise RuntimeError("Invalid rttest file {}: Expected group end "
                "marker at byte {}".format(rttpath, f.tell() - 1))
        return False

    def readByte(self, f, rttpath):
        byte = f.read(1)
        if byte == "":
            raise RuntimeError("Invalid rttest file {}: "
                "Unexpected end-of-file".format(rttpath))
        return byte


# Parse arguments
argsParser = argparse.ArgumentParser(description="Parses an rttest output file and pretty-prints it in a text format")
argsParser.add_argument("--verbose", action='store_true', default=False,
    help="Print PASS test cases as well as FAIL ones")
argsParser.add_argument("RTTFILE", help="Output file from the rttest run")
argsParser.add_argument("SRCFILE", nargs='+', help="Unit test C source files")
args = argsParser.parse_args()

# Build map of test groups
mapbuilder = MapBuilder()
rttmap = mapbuilder.Build(args.SRCFILE)
#mapbuilder.Dump(rttmap)

rttparser = RttParser()
if not rttparser.Parse(args.RTTFILE, rttmap, args.verbose):
    exit(1)
exit(0)
