#!/usr/bin/env python

import os
import subprocess
import fnmatch


###############
# Test rttest #
###############

paths = ["build/x64-linux/release/rttest_unit_tests",
         "build/x64-linux/debug/rttest_unit_tests"]
path = ""
for p in paths:
    if os.access(p, os.R_OK):
        path = p
        break

if path == "":
    raise RuntimeError("Please compile rttest unit tests; paths tried: {}".format(paths))

# Run the rttest unit tests and capture the output
output = subprocess.check_output([path])

# Compare the actual output with what is expected
f = open("src/rttest/test/expected.rtt", 'r')
expected = f.read()
if expected != output:
    print("FAIL rttest - output: '{}', expected '{}'".format(output, expected))
    exit(1)


##############
# Test rtsys #
##############

modes = ["release", "debug"]
mode = ""
path = ""
for m in modes:
    path = "build/x64-linux/" + m + "/rtsys_unit_tests"
    if os.access(path, os.R_OK):
        mode = m
        break

if mode == "":
    raise RuntimeError("Please compile rtsys unit tests; modes tried: {}".format(modes))

# Run the rtsys unit tests and capture the output
output = subprocess.check_output([path])
f = open("rtsys.rtt", 'w')
f.write(output)
f.close()

# Find all the unit test source files
unitTests = []
for dName, dirList, fileList in os.walk("."):
    if dName != "." and dName != "./build":
        for f in fileList:
            if fnmatch.fnmatch(f, "unittest*.c"):
                unitTests.append(dName + "/" + f)

# Print the results
ret = 0
try:
    args = ["src/rttest/scripts/rttest2text.py", "rtsys.rtt"]
    args.extend(unitTests)
    subprocess.check_call(args)
except:
    ret = 1
os.unlink("rtsys.rtt")
exit(ret)
