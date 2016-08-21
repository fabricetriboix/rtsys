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

/** Test framework
 *
 * @defgroup rttest Test framework
 * @addtogroup rttest
 * @{
 *
 * Calling `RTTestRun()` will run all test groups and will write the results as
 * it goes along to the output. You have to provide a function to write to the
 * output, please refer to `RTTestOutputChar`.
 */

/** Example code

static RTBool myGroupEntry(void)
{
    RTBool ok = RTTrue;
    if (init() < 0) {
        ok = RTFalse;
    }
    return ok;
}

static RTBool myGroupExit(void)
{
    RTBool ok = RTTrue;
    if (deinit() < 0) {
        ok = RTFalse;
    }
    return ok;
}

RTT_GROUP_START(MyGroup, 0x00001001u, myGroupEntry, myGroupExit)

RTT_TEST_START(first_test_case)
{
    RTT_EXPECT(thisShouldBeTrue());
    RTT_ASSERT(!thisMustBeFalse());
}
RTT_TEST_END

RTT_TEST_START(second_test_case)
{
    RTT_EXPECT(doSomeWork() >= 0);
}
RTT_TEST_END

RTT_GROUP_END(MyGroup, first_test_case, second_test_case)

*/


#ifndef RTTEST_h_
#define RTTEST_h_

#include "rtplf.h"
#include "rttest_priv.h"



/*-------+
 | Types |
 +-------*/


/** "Opaque" type to a test group */
typedef struct RTPrivTestGroup RTTestGroup;


/** Prototype of a function to write an octet to the output
 *
 * As an exception to the philosophy of these software modules, this function is
 * allowed to block. This is to allow transmission of test results over a slow
 * output (eg: slow serial port). The assumption here is that blocking between
 * tests will be short and will not have an adverse effect.
 *
 * The function should return `RTTrue` if the octet has been written, and
 * `RTFalse` if not, in which case the execution of the tests will stop
 * immediately.
 */
typedef RTBool (*RTTestWriteOctet)(uint8_t octet);



/*--------+
 | Macros |
 +--------*/


/** Start a test group
 *
 * @param _name  [in] Test group name, do not enclose in quotes
 * @param _id    [in] Numerical identifier uniquely identifying this test group.
 *                    This number must fit an `uint32_t`.
 * @param _entry [in] Entry action, can be NULL if no entry action; see below
 * @param _exit  [in] Exit action, can be NULL if no entry action; see below
 *
 * The entry and exit actions must be functions with the following prototype:
 *
 *     RTBool entry_or_exit_action(void);
 *
 * They must return `RTTrue` if the test group has been successfully
 * initialised/de-initialised. Otherwise, they should return `RTFalse` and the
 * execution of the tests will be immediately stopped.
 */
#define RTT_GROUP_START(_name, _id, _entry, _exit) \
    RTTPRIV_GROUP_START(_name, _id, _entry, _exit)


/** Start a test case
 *
 * @param _name [in] Test case name, do not enclose in quotes. `_name` must be
 *                   unique across all the test groups. `_name` will be used as
 *                   a compiler symbol, so it must be unique within the limits
 *                   of your compiler. For example, if your compiler determines
 *                   uniqueness of a symbol based on the first 32 characters
 *                   only, then the first 32 characters of `_name` must be
 *                   unique across the whole compilation artefact.
 */
#define RTT_TEST_START(_name) RTTPRIV_TEST_START(_name)


/** End a test case previously started with `RTT_TEST_START()` */
#define RTT_TEST_END RTTPRIV_TEST_END


/** End a test group previously started with `RTT_GROUP_START()`
 *
 * @param _name [in] The test group name; this must be exactly the same as the
 *                   one used in the call to `RTT_GROUP_START()`.
 * @param ...   [in] Comma-separated list of test cases to include in this
 *                   group. These must be the exact names used in previous calls
 *                   to `RTT_TEST_START()`.
 */
#define RTT_GROUP_END(_name, ...) RTTPRIV_GROUP_END(_name, __VA_ARGS__)


/** Fail a test case if a condition is not true
 *
 * @param _cond [in] The condition to test. If `_cond` evaluates to false,
 *                   this test case is considered failed and no more code from
 *                   this test case will be executed.
 */
#define RTT_EXPECT(_cond) RTTPRIV_EXPECT(_cond)


/** Fatally fail a test case if a condition is not true
 *
 * @param _cond [in] The condition to test. If `_cond` evaluates to false,
 *                   this test case is considered failed and no more code from
 *                   this test case will be executed. In addition, no more
 *                   test cases in this group will be executed. Other test
 *                   groups will still be run though.
 */
#define RTT_ASSERT(_cond) RTTPRIV_ASSERT(_cond)



/*------------------------------+
 | Public function declarations |
 +------------------------------*/


/** Run the tests
 *
 * Calling this function will run the tests.
 *
 * @param wrOctet [in] Function to write an octet to the output; must not be
 *                     NULL.
 * @param groups  [in] Array of numerical identifiers of test groups to run. Set
 *                     to NULL to run all test groups.
 * @param ngroups [in] Size of the `groups` array. Set to 0 to run all test
 *                     groups.
 *
 * @return * The number of failed test cases (0 if all the tests passed)
 *         * -1 if a call to `outch()` failed
 *         * -2 if a call to a group entry action failed
 *         * -3 if a call to a group exit action failed
 */
int32_t RTTestRun(RTTestWriteOctet wrOctet,
        const uint32_t* groups, uint16_t ngroups);


#endif /* RTTEST_h_ */
/* @} */
