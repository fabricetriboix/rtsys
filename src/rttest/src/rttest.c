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


#include "rtplf.h"
#include "rttest.h"
#include "rttest_format.h"



/*------------------+
 | Global variables |
 +------------------*/


static struct RTPrivTestGroup* gFirstGroup = NULL;



/*-------------------------------+
 | Private function declarations |
 +-------------------------------*/


/** Write the test results preamble
 *
 * @param wrOctet [in] Function to output an octet; must not be NULL
 *
 * @return 0 if OK, -1 if invalid argument, -2 if a call to `wrOctet()` failed
 */
static int32_t rttestWritePreamble(RTTestWriteOctet wrOctet);


/** Check if the given number is in the given set
 *
 * @param x    [in] The number to check
 * @param set  [in] The set to check. `set` may be NULL, in which case `x` is
 *                  considered to be in it.
 * @param size [in] Size of the `set` array. `size` may be 0, in which case `x`
 *                  is considered to be in the `set`.
 */
static RTBool rttestIsIn(uint32_t x, const uint32_t* set, uint16_t size);


/** Run a test group
 *
 * If a test case asserts, the execution of the test cases is immediately
 * stopped, and all remaining test cases are considered to have failed.
 *
 * @param wrOctet [in] Function to output an octet; must not be NULL
 * @param group   [in] The test group to run; must not be NULL
 *
 * @return * The number of failed test cases (0 if all the tests passed)
 *         * -1 if a call to `wrOctet()` failed
 *         * -2 if a call to a group entry action failed
 *         * -3 if a call to a group exit action failed
 */
static int32_t rttestRunGroup(RTTestWriteOctet wrOctet,
        const struct RTPrivTestGroup* group);


/** Run the test cases of a test group
 *
 * @param wrOctet [in] Function to output an octet; must not be NULL
 * @param group   [in] The test group to run; must not be NULL
 *
 * @return * The number of failed test cases (0 if all the tests passed)
 *         * -1 if a call to `wrOctet()` failed
 *         * -2 if a call to a group entry action failed
 *         * -3 if a call to a group exit action failed
 */
static int32_t rttestRunTestCases(RTTestWriteOctet wrOctet,
        const RTTestGroup* group);



/*---------------------------------+
 | Public function implementations |
 +---------------------------------*/


void RTPrivTestRegisterGroup(struct RTPrivTestGroup* group)
{
    RTASSERT(group != NULL);

    if (gFirstGroup == NULL) {
        gFirstGroup = group;
    } else {
        struct RTPrivTestGroup* i;
        for (i = gFirstGroup; i->next != NULL; i = i->next) {
            RTASSERT(i->id != group->id);
        }
        i->next = group;
        group->next = NULL;
    }
}


int32_t RTTestRun(RTTestWriteOctet wrOctet,
        const uint32_t* groups, uint16_t ngroups)
{
    int32_t ret;
    struct RTPrivTestGroup* group;

    RTASSERT(wrOctet != NULL);
    RTASSERT(gFirstGroup != NULL);

    ret = rttestWritePreamble(wrOctet);
    if (ret >= 0) {
        for (group = gFirstGroup; group != NULL; group = group->next) {
            if (rttestIsIn(group->id, groups, ngroups)) {
                ret = rttestRunGroup(wrOctet, group);
                if (ret < 0) {
                    break;
                }
            }
        }
        if (ret >= 0) {
            if (!wrOctet((uint8_t)RTTEST_END_OF_FILE)) {
                ret = -1;
            }
        }
    }
    return ret;
}



/*----------------------------------+
 | Private function implementations |
 +----------------------------------*/


static int32_t rttestWritePreamble(RTTestWriteOctet wrOctet)
{
    int32_t ret = 0;
    uint8_t i;
    uint8_t fourcc[4];

    RTASSERT(wrOctet != NULL);

    RTMemcpy(fourcc, sizeof(fourcc), (const uint8_t*)RTTEST_FOUR_CC, 4);
    for (i = 0; i < sizeof(fourcc); i++) {
        if (!wrOctet(fourcc[i])) {
            ret = -1;
            break;
        }
    }
    return ret;
}


static RTBool rttestIsIn(uint32_t x, const uint32_t* set, uint16_t size)
{
    RTBool found = RTTrue;

    if ((set != NULL) && (size > 0)) {
        uint16_t i;
        found = RTFalse;
        for (i = 0; (i < size) && !found; i++) {
            if (x == set[i]) {
                found = RTTrue;
            }
        }
    }
    return found;
}


static int32_t rttestRunGroup(RTTestWriteOctet wrOctet,
        const struct RTPrivTestGroup* group)
{
    int32_t ret = 0;
    uint32_t id;
    int8_t i;

    /* Check arguments */
    RTASSERT(wrOctet != NULL);
    RTASSERT(group != NULL);

    /* Output: we are entering a group */
    if (!wrOctet((uint8_t)RTTEST_ENTER_GROUP)) {
        ret = -1;
    }

    /* Output group id in big-endian format */
    id = group->id;
    for (i = 0; i < 4; i++) {
        uint8_t tmp = id >> 24;
        if (!wrOctet(tmp)) {
            ret = -1;
            break;
        }
        id <<= 8;
    }

    /* Execute group entry action, if any */
    if ((ret >= 0) && (group->entryAction != NULL)) {
        if (!(group->entryAction())) {
            ret = -2;
        }
    }

    /* Execute test cases */
    if (ret >= 0) {
        ret = rttestRunTestCases(wrOctet, group);
    }

    /* Execute group exit action, if any */
    if ((ret >= 0) && (group->exitAction != NULL)) {
        if (!(group->exitAction())) {
            ret = -3;
        }
    }

    /* Output: we are exiting a group */
    if (ret >= 0) {
        if (!wrOctet((uint8_t)RTTEST_EXIT_GROUP)) {
            ret = -1;
        }
    }

    return ret;
}


static int32_t rttestRunTestCases(RTTestWriteOctet wrOctet,
        const RTTestGroup* group)
{
    int32_t ret = 0;
    RTBool asserted = RTFalse;
    uint8_t resultBits = 0;
    uint32_t i;

    RTASSERT(wrOctet != NULL);
    RTASSERT(group != NULL);
    RTASSERT(group->testCases[0] != NULL); /* at least one test case */

    for (i = 0; group->testCases[i] != NULL; i++) {
        RTBool failed = RTFalse;
        if (asserted) {
            /* A previous test case asserted => Assume this one failed */
            failed = RTTrue;
        } else {
            int8_t result = group->testCases[i]();
            if (result < 0) {
                failed = RTTrue;
                asserted = RTTrue;
            } else if (result > 0) {
                failed = RTTrue;
            }
        }
        if (failed) {
            resultBits |= 1u << (i % 8);
            ret++;
        }
        if (((i + 1) % 8) == 0) {
            if (!wrOctet(resultBits)) {
                ret = -1;
                break;
            }
            resultBits = 0;
        }
    } /* For each test case in this group */

    if ((i % 8) != 0) {
        if (!wrOctet(resultBits)) {
            ret = -1;
        }
    }
    return ret;
}
