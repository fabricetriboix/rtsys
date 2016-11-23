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

#include "rtfifo.h"
#include "rttest.h"
#include "rtplf.h"


typedef struct
{
    uint16_t a;
    int8_t b;
} TSmallItem;

static TSmallItem gSmallBuffer[10];
static RTSmallFifo gSmallFifo = RT_SMALL_FIFO_INIT(gSmallBuffer);

RTT_GROUP_START(TestSmallFifo, 0x00020001u, NULL, NULL)

RTT_TEST_START(smallfifo_should_be_empty_after_creation)
{
    RTT_ASSERT(RTSmallFifoIsEmpty(&gSmallFifo));
}
RTT_TEST_END

RTT_TEST_START(smallfifo_should_not_be_full_after_creation)
{
    RTT_ASSERT(!RTSmallFifoIsFull(&gSmallFifo));
}
RTT_TEST_END

RTT_TEST_START(smallfifo_size_should_be_0_after_creation)
{
    RTT_ASSERT(RTSmallFifoSize(&gSmallFifo) == 0);
}
RTT_TEST_END

RTT_TEST_START(smallfifo_capacity_should_be_10_after_creation)
{
    RTT_ASSERT(RTSmallFifoCapacity(&gSmallFifo) == 10u);
}
RTT_TEST_END

RTT_TEST_START(smallfifo_should_not_pop_after_creation)
{
    TSmallItem item;
    RTT_ASSERT(!RTSmallFifoPop(&gSmallFifo, &item, sizeof(item)));
}
RTT_TEST_END

RTT_TEST_START(smallfifo_should_push_7_items)
{
    TSmallItem item;
    uint16_t i;

    item.a = 99u;
    item.b = -5;
    for (i = 0; i < 7u; i++) {
        item.a++;
        item.b--;
        RTT_ASSERT(RTSmallFifoPush(&gSmallFifo, &item, sizeof(item)));
    }
}
RTT_TEST_END

RTT_TEST_START(smallfifo_should_pop_3_items)
{
    TSmallItem ref;
    TSmallItem item;
    uint16_t i;

    ref.a = 99u;
    ref.b = -5;
    for (i = 0; i < 3u; i++) {
        ref.a++;
        ref.b--;
        RTT_ASSERT(RTSmallFifoPop(&gSmallFifo, &item, sizeof(item)));
        RTT_EXPECT((item.a == ref.a) && (item.b == ref.b));
    }
}
RTT_TEST_END

RTT_TEST_START(smallfifo_should_push_6_items)
{
    TSmallItem item;
    uint16_t i;

    item.a = 2003u;
    item.b = -90;
    for (i = 0; i < 6u; i++) {
        item.a--;
        item.b++;
        RTT_ASSERT(RTSmallFifoPush(&gSmallFifo, &item, sizeof(item)));
    }
}
RTT_TEST_END

RTT_TEST_START(smallfifo_should_be_full_when_filled_up)
{
    RTT_ASSERT(RTSmallFifoIsFull(&gSmallFifo));
}
RTT_TEST_END

RTT_TEST_START(smallfifo_size_should_be_10_when_filled_up)
{
    RTT_ASSERT(RTSmallFifoSize(&gSmallFifo) == 10u);
}
RTT_TEST_END

RTT_TEST_START(smallfifo_capacity_should_be_10_when_filled_up)
{
    RTT_ASSERT(RTSmallFifoCapacity(&gSmallFifo) == 10);
}
RTT_TEST_END

RTT_TEST_START(smallfifo_should_fail_to_push_when_filled_up)
{
    TSmallItem item;
    RTT_ASSERT(!RTSmallFifoPush(&gSmallFifo, &item, sizeof(item)));
}
RTT_TEST_END

RTT_TEST_START(smallfifo_should_pop_4_items)
{
    TSmallItem ref;
    TSmallItem item;
    uint16_t i;

    ref.a = 102u;
    ref.b = -8;
    for (i = 0; i < 4u; i++) {
        ref.a++;
        ref.b--;
        RTT_ASSERT(RTSmallFifoPop(&gSmallFifo, &item, sizeof(item)));
        RTT_EXPECT((item.a == ref.a) && (item.b == ref.b));
    }
}
RTT_TEST_END

RTT_TEST_START(smallfifo_should_not_be_empty_when_partially_full)
{
    RTT_ASSERT(!RTSmallFifoIsEmpty(&gSmallFifo));
}
RTT_TEST_END

RTT_TEST_START(smallfifo_should_not_be_full_when_partially_full)
{
    RTT_ASSERT(!RTSmallFifoIsFull(&gSmallFifo));
}
RTT_TEST_END

RTT_TEST_START(smallfifo_size_should_be_6_when_partially_full)
{
    RTT_ASSERT(RTSmallFifoSize(&gSmallFifo) == 6u);
}
RTT_TEST_END

RTT_TEST_START(smallfifo_capacity_should_be_10_when_partially_full)
{
    RTT_ASSERT(RTSmallFifoCapacity(&gSmallFifo) == 10u);
}
RTT_TEST_END

RTT_TEST_START(smallfifo_should_pop_6_items)
{
    TSmallItem ref;
    TSmallItem item;
    uint16_t i;

    ref.a = 2003u;
    ref.b = -90;
    for (i = 0; i < 6u; i++) {
        ref.a--;
        ref.b++;
        RTT_ASSERT(RTSmallFifoPop(&gSmallFifo, &item, sizeof(item)));
        RTT_EXPECT((item.a == ref.a) && (item.b == ref.b));
    }
}
RTT_TEST_END

RTT_TEST_START(smallfifo_should_be_empty_when_emptied)
{
    RTT_ASSERT(RTSmallFifoIsEmpty(&gSmallFifo));
}
RTT_TEST_END

RTT_GROUP_END(TestSmallFifo,
        smallfifo_should_be_empty_after_creation,
        smallfifo_should_not_be_full_after_creation,
        smallfifo_size_should_be_0_after_creation,
        smallfifo_capacity_should_be_10_after_creation,
        smallfifo_should_not_pop_after_creation,
        smallfifo_should_push_7_items,
        smallfifo_should_pop_3_items,
        smallfifo_should_push_6_items,
        smallfifo_should_be_full_when_filled_up,
        smallfifo_size_should_be_10_when_filled_up,
        smallfifo_capacity_should_be_10_when_filled_up,
        smallfifo_should_fail_to_push_when_filled_up,
        smallfifo_should_pop_4_items,
        smallfifo_should_not_be_empty_when_partially_full,
        smallfifo_should_not_be_full_when_partially_full,
        smallfifo_size_should_be_6_when_partially_full,
        smallfifo_capacity_should_be_10_when_partially_full,
        smallfifo_should_pop_6_items,
        smallfifo_should_be_empty_when_emptied)


typedef struct
{
    uint32_t a;
    int32_t b;
    RTByte stuff[300];
} TItem;

static TItem gBuffer[1000];
static RTFifo gFifo;

static RTBool TestFifoEntry(void)
{
    RTFifoInit(&gFifo, RTARRAYSIZE(gBuffer), sizeof(gBuffer[0]),
            (RTByte*)gBuffer);
    return RTTrue;
}

RTT_GROUP_START(TestFifo, 0x00020002u, TestFifoEntry, NULL)

RTT_TEST_START(fifo_should_be_empty_after_creation)
{
    RTT_ASSERT(RTFifoIsEmpty(&gFifo));
}
RTT_TEST_END

RTT_TEST_START(fifo_should_not_be_full_after_creation)
{
    RTT_ASSERT(!RTFifoIsFull(&gFifo));
}
RTT_TEST_END

RTT_TEST_START(fifo_size_should_be_0_after_creation)
{
    RTT_ASSERT(RTFifoSize(&gFifo) == 0);
}
RTT_TEST_END

RTT_TEST_START(fifo_capacity_should_be_1000_after_creation)
{
    RTT_ASSERT(RTFifoCapacity(&gFifo) == 1000u);
}
RTT_TEST_END

RTT_TEST_START(fifo_should_not_pop_after_creation)
{
    TItem item;
    RTT_ASSERT(!RTFifoPop(&gFifo, &item, sizeof(item)));
}
RTT_TEST_END

RTT_TEST_START(fifo_should_push_700_items)
{
    TItem item;
    uint16_t i;

    item.a = 99887766u;
    item.b = -5;
    for (i = 0; i < 700u; i++) {
        item.a++;
        item.b--;
        RTT_ASSERT(RTFifoPush(&gFifo, &item, sizeof(item)));
    }
}
RTT_TEST_END

RTT_TEST_START(fifo_should_pop_300_items)
{
    TItem ref;
    TItem item;
    uint16_t i;

    ref.a = 99887766u;
    ref.b = -5;
    for (i = 0; i < 300u; i++) {
        ref.a++;
        ref.b--;
        RTT_ASSERT(RTFifoPop(&gFifo, &item, sizeof(item)));
        RTT_EXPECT((item.a == ref.a) && (item.b == ref.b));
    }
}
RTT_TEST_END

RTT_TEST_START(fifo_should_push_600_items)
{
    TItem item;
    uint16_t i;

    item.a = 2003u;
    item.b = -90;
    for (i = 0; i < 600u; i++) {
        item.a--;
        item.b++;
        RTT_ASSERT(RTFifoPush(&gFifo, &item, sizeof(item)));
    }
}
RTT_TEST_END

RTT_TEST_START(fifo_should_be_full_when_filled_up)
{
    RTT_ASSERT(RTFifoIsFull(&gFifo));
}
RTT_TEST_END

RTT_TEST_START(fifo_size_should_be_1000_when_filled_up)
{
    RTT_ASSERT(RTFifoSize(&gFifo) == 1000u);
}
RTT_TEST_END

RTT_TEST_START(fifo_capacity_should_be_1000_when_filled_up)
{
    RTT_ASSERT(RTFifoCapacity(&gFifo) == 1000u);
}
RTT_TEST_END

RTT_TEST_START(fifo_should_fail_to_push_when_filled_up)
{
    TItem item;
    RTT_ASSERT(!RTFifoPush(&gFifo, &item, sizeof(item)));
}
RTT_TEST_END

RTT_TEST_START(fifo_should_pop_400_items)
{
    TItem ref;
    TItem item;
    uint16_t i;

    ref.a = 99888066u;
    ref.b = -305;
    for (i = 0; i < 400u; i++) {
        ref.a++;
        ref.b--;
        RTT_ASSERT(RTFifoPop(&gFifo, &item, sizeof(item)));
        RTT_EXPECT((item.a == ref.a) && (item.b == ref.b));
    }
}
RTT_TEST_END

RTT_TEST_START(fifo_should_not_be_empty_when_partially_full)
{
    RTT_ASSERT(!RTFifoIsEmpty(&gFifo));
}
RTT_TEST_END

RTT_TEST_START(fifo_should_not_be_full_when_partially_full)
{
    RTT_ASSERT(!RTFifoIsFull(&gFifo));
}
RTT_TEST_END

RTT_TEST_START(fifo_size_should_be_600_when_partially_full)
{
    RTT_ASSERT(RTFifoSize(&gFifo) == 600u);
}
RTT_TEST_END

RTT_TEST_START(fifo_capacity_should_be_1000_when_partially_full)
{
    RTT_ASSERT(RTFifoCapacity(&gFifo) == 1000u);
}
RTT_TEST_END

RTT_TEST_START(fifo_should_pop_600_items)
{
    TItem ref;
    TItem item;
    uint16_t i;

    ref.a = 2003u;
    ref.b = -90;
    for (i = 0; i < 600u; i++) {
        ref.a--;
        ref.b++;
        RTT_ASSERT(RTFifoPop(&gFifo, &item, sizeof(item)));
        RTT_EXPECT((item.a == ref.a) && (item.b == ref.b));
    }
}
RTT_TEST_END

RTT_TEST_START(fifo_should_be_empty_when_emptied)
{
    RTT_ASSERT(RTFifoIsEmpty(&gFifo));
}
RTT_TEST_END

RTT_GROUP_END(TestFifo,
        fifo_should_be_empty_after_creation,
        fifo_should_not_be_full_after_creation,
        fifo_size_should_be_0_after_creation,
        fifo_capacity_should_be_1000_after_creation,
        fifo_should_not_pop_after_creation,
        fifo_should_push_700_items,
        fifo_should_pop_300_items,
        fifo_should_push_600_items,
        fifo_should_be_full_when_filled_up,
        fifo_size_should_be_1000_when_filled_up,
        fifo_capacity_should_be_1000_when_filled_up,
        fifo_should_fail_to_push_when_filled_up,
        fifo_should_pop_400_items,
        fifo_should_not_be_empty_when_partially_full,
        fifo_should_not_be_full_when_partially_full,
        fifo_size_should_be_600_when_partially_full,
        fifo_capacity_should_be_1000_when_partially_full,
        fifo_should_pop_600_items,
        fifo_should_be_empty_when_emptied)
