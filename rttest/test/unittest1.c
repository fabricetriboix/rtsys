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
#include "rtplf.h"
#include "testme.h"


static RTBool dummyGroupEntry(void)
{
    return RTTrue;
}

static RTBool dummyGroupExit(void)
{
    return RTTrue;
}

RTT_GROUP_START(LessThan, 0x00001001u, dummyGroupEntry, dummyGroupExit)

RTT_TEST_START(four_should_be_less_than_five)
{
    /* This test should fail */
    RTT_EXPECT(compare(4, 5) < 0);
}
RTT_TEST_END

RTT_TEST_START(minus_46_should_be_less_than_minus_37)
{
    /* This test should fail */
    RTT_EXPECT(compare(-46, -37) < 0);
}
RTT_TEST_END

RTT_TEST_START(minus_100000_should_be_less_than_7)
{
    /* This test should fail */
    RTT_EXPECT(compare(-100000, 7) < 0);
}
RTT_TEST_END

RTT_GROUP_END(LessThan,
        four_should_be_less_than_five,
        minus_46_should_be_less_than_minus_37,
        minus_100000_should_be_less_than_7)


RTT_GROUP_START(TestAssert, 0x00001002u, NULL, NULL)

RTT_TEST_START(should_assert)
{
    /* This test should fail */
    RTT_ASSERT(compare(5, 9) < 0);
}
RTT_TEST_END

RTT_TEST_START(should_not_be_run)
{
    /* This test case should not be run because the previous one asserted */
    RTPanic();
}
RTT_TEST_END

RTT_GROUP_END(TestAssert,
        should_assert,
        should_not_be_run)


RTT_GROUP_START(EqualTo, 0x00001003u, NULL, NULL)

RTT_TEST_START(seventy_eight_should_be_equal_to_seventy_eight)
{
    /* This test should fail */
    RTT_EXPECT(compare(78, 78) == 0);
}
RTT_TEST_END

RTT_TEST_START(minus_98457_should_be_equal_to_minus_98457)
{
    /* This test should fail */
    RTT_EXPECT(compare(-98457, -98457) == 0);
}
RTT_TEST_END

RTT_GROUP_END(EqualTo,
        seventy_eight_should_be_equal_to_seventy_eight,
        minus_98457_should_be_equal_to_minus_98457)
