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

/* You should not include this file directly; include "rttest.h" instead */


#ifndef RTTEST_PRIV_h_
#define RTTEST_PRIV_h_


#include "rtplf.h"



/*-------+
 | Types |
 +-------*/


/** Prototype for a group entry/exit action function
 *
 * The function must return `RTTrue` if it successfully
 * initialised/de-initialised the test group. Otherwise, it should return
 * `RTFalse` and the execution of the tests will be immediately stopped.
 */
typedef RTBool (*RTPrivTestAction)(void);


/** Prototype for a test case function
 *
 * This function must return 0 if the test case has been passed successfully.
 *
 * If the test case fails, but the rest of the test group can still be run, this
 * function should return 1.
 *
 * If the test case fails, and it does not make sense to continue the test
 * group, this function should return -1.
 */
typedef int8_t (*RTPrivTestCase)(void);


/** Structure that defines a test group */
struct RTPrivTestGroup
{
    struct RTPrivTestGroup* next; /**< Pointer to next test group, or NULL if this is the last group */
    uint32_t         id;          /**< Unique identifier for this test group */
    RTPrivTestAction entryAction; /**< Entry action, or NULL if no action */
    RTPrivTestAction exitAction;  /**< Exit action, or NULL if no action */
    RTPrivTestCase*  testCases;   /**< NULL-terminated array of test cases */
};



/*--------+
 | Macros |
 +--------*/


#define RTTPRIV_GROUP_START(_name, _id, _entry, _exit) \
    extern RTPrivTestCase RTTC##_name[]; \
    static struct RTPrivTestGroup RTTG##_name = { \
        NULL, \
        (_id), \
        (_entry), \
        (_exit), \
        RTTC##_name \
    };


#define RTTPRIV_TEST_START(_name) \
    static int8_t _name(void) \
    {


#define RTTPRIV_TEST_END \
        return 0; \
    }


#define RTTPRIV_EXPECT(_cond) \
    do { \
        if (!(_cond)) { \
            return 1; \
        } \
    } while (0)


#define RTTPRIV_ASSERT(_cond) \
    do { \
        if (!(_cond)) { \
            return -1; \
        } \
    } while (0)


#define RTTPRIV_GROUP_END(_name, ...) \
    RTPrivTestCase RTTC##_name[] = { __VA_ARGS__ , NULL }; \
    static void RTTR##_name(void) __attribute__((constructor)); \
    static void RTTR##_name(void) \
    { \
        RTPrivTestRegisterGroup(&RTTG##_name); \
    }



/*------------------------------+
 | Public function declarations |
 +------------------------------*/


/** Register a test group
 *
 * \param group [in] Group to register; must not be NULL. `group->mId` must be
 *                   different from all the other groups already registered.
 */
void RTPrivTestRegisterGroup(struct RTPrivTestGroup* group);



#endif /* RTTEST_PRIV_h_ */
/* @} */
