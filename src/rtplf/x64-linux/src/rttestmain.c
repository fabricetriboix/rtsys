/* Copyright (c) 2014-2016  Fabrice Triboix
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rttest.h"
#include <stdlib.h>
#include <unistd.h>


static RTBool writeOctet(uint8_t octet)
{
    write(STDOUT_FILENO, &octet, 1);
    return RTTrue;
}


int main(int argc, char** argv)
{
    int32_t ret;
    uint32_t* groups = NULL;
    uint16_t ngroups = 0;

    if (argc > 1) {
        uint16_t i;
        ngroups = (uint16_t)(argc - 1);
        groups = calloc(ngroups, sizeof(uint32_t));
        RTASSERT(groups != NULL);
        for (i = 0; i < ngroups; i++) {
            RTBool ok = RTStringToU32(argv[i + 1], RTBASE_AUTO, &groups[i]);
            RTASSERT(ok);
        }
    }
    ret = RTTestRun(writeOctet, groups, ngroups);
    free(groups);

    if (ret < 0) {
        ret = 1;
    } else {
        ret = 0;
    }
    return ret;
}
