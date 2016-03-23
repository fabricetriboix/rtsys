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


RTT_GROUP_START(MoreThan, 0x00002001u, NULL, NULL)

RTT_TEST_START(two_hundred_thirty_four_should_be_more_than_ninety_eight)
{
    /* This test should succeed */
    RTT_EXPECT(compare(234, -98) > 0);
}
RTT_TEST_END

RTT_TEST_START(minus_456_should_be_more_than_minus_457)
{
    /* This test should succeed */
    RTT_EXPECT(compare(-456, -457) > 0);
}
RTT_TEST_END

RTT_TEST_START(filler_2)
{
    RTT_EXPECT(RTFalse);
}
RTT_TEST_END

RTT_TEST_START(filler_3)
{
}
RTT_TEST_END

RTT_TEST_START(filler_4)
{
    RTT_EXPECT(RTFalse);
}
RTT_TEST_END

RTT_TEST_START(filler_5)
{
}
RTT_TEST_END

RTT_TEST_START(filler_6)
{
    RTT_EXPECT(RTFalse);
}
RTT_TEST_END

RTT_TEST_START(filler_7)
{
}
RTT_TEST_END

RTT_TEST_START(filler_8)
{
}
RTT_TEST_END

RTT_TEST_START(filler_9)
{
    RTT_EXPECT(RTFalse);
}
RTT_TEST_END

RTT_TEST_START(filler_10)
{
    RTT_EXPECT(RTFalse);
}
RTT_TEST_END

RTT_GROUP_END(MoreThan,
        two_hundred_thirty_four_should_be_more_than_ninety_eight,
        minus_456_should_be_more_than_minus_457,
        filler_2,
        filler_3,
        filler_4,
        filler_5,
        filler_6,
        filler_7,
        filler_8,
        filler_9,
        filler_10)
