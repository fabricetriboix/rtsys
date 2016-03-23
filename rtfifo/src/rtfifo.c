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
#include "rtfifo.h"



/*---------------------------------+
 | Public function implementations |
 +---------------------------------*/


void RTSmallFifoInit(RTSmallFifo* fifo, uint8_t capacity,
        uint8_t itemSize_B, RTByte* buffer)
{
    RTASSERT(fifo != NULL);
    RTASSERT(capacity > 0);
    RTASSERT(itemSize_B > 0);
    RTASSERT(buffer != NULL);

    fifo->head = 0;
    fifo->tail = 0;
    fifo->size = 0;
    fifo->capacity = capacity;
    fifo->itemSize_B = itemSize_B;
    fifo->buffer = buffer;
}


uint8_t RTSmallFifoSize(const RTSmallFifo* fifo)
{
    RTASSERT(fifo != NULL);
    return fifo->size;
}


uint8_t RTSmallFifoCapacity(const RTSmallFifo* fifo)
{
    RTASSERT(fifo != NULL);
    return fifo->capacity;
}


RTBool RTSmallFifoIsEmpty(const RTSmallFifo* fifo)
{
    RTASSERT(fifo != NULL);
    return fifo->size == 0;
}


RTBool RTSmallFifoIsFull(const RTSmallFifo* fifo)
{
    RTASSERT(fifo != NULL);
    return (fifo->size >= fifo->capacity);
}


RTBool RTSmallFifoPush(RTSmallFifo* fifo, const void* item, uint8_t itemSize_B)
{
    RTBool pushed = RTFalse;

    RTASSERT(fifo != NULL);
    RTASSERT(fifo->buffer != NULL);
    RTASSERT(item != NULL);
    RTASSERT(itemSize_B > 0);
    RTASSERT(itemSize_B <= fifo->itemSize_B);

    if (fifo->size < fifo->capacity) {
        RTByte* dst = &(fifo->buffer[fifo->head * fifo->itemSize_B]);
        RTMemcpy(dst, fifo->itemSize_B, item, itemSize_B);

        /* Increment head */
        fifo->head++;
        if (fifo->head >= fifo->capacity) {
            fifo->head = 0;
        }
        fifo->size++;
        pushed = RTTrue;
    }
    return pushed;
}


RTBool RTSmallFifoPop(RTSmallFifo* fifo, void* item, uint8_t itemSize_B)
{
    RTBool popped = RTFalse;

    RTASSERT(fifo != NULL);
    RTASSERT(fifo->buffer != NULL);
    RTASSERT(item != NULL);
    RTASSERT(itemSize_B > 0);
    RTASSERT(itemSize_B >= fifo->itemSize_B);

    if (fifo->size > 0) {
        const RTByte* src = &(fifo->buffer[fifo->tail * fifo->itemSize_B]);
        RTMemcpy(item, itemSize_B, src, fifo->itemSize_B);

        /* Increment tail */
        fifo->tail++;
        if (fifo->tail >= fifo->capacity) {
            fifo->tail = 0;
        }
        fifo->size--;
        popped = RTTrue;
    }
    return popped;
}


void RTFifoInit(RTFifo* fifo, uint16_t capacity,
        uint16_t itemSize_B, RTByte* buffer)
{
    RTASSERT(fifo != NULL);
    RTASSERT(capacity > 0);
    RTASSERT(itemSize_B > 0);
    RTASSERT(buffer != NULL);

    fifo->head = 0;
    fifo->tail = 0;
    fifo->size = 0;
    fifo->capacity = capacity;
    fifo->itemSize_B = itemSize_B;
    fifo->buffer = buffer;
}


uint16_t RTFifoSize(const RTFifo* fifo)
{
    RTASSERT(fifo != NULL);
    return fifo->size;
}


uint16_t RTFifoCapacity(const RTFifo* fifo)
{
    RTASSERT(fifo != NULL);
    return fifo->capacity;
}


RTBool RTFifoIsEmpty(const RTFifo* fifo)
{
    RTASSERT(fifo != NULL);
    return fifo->size == 0;
}


RTBool RTFifoIsFull(const RTFifo* fifo)
{
    RTASSERT(fifo != NULL);
    return fifo->size >= fifo->capacity;
}


RTBool RTFifoPush(RTFifo* fifo, const void* item, uint16_t itemSize_B)
{
    RTBool pushed = RTFalse;

    RTASSERT(fifo != NULL);
    RTASSERT(fifo->buffer != NULL);
    RTASSERT(item != NULL);
    RTASSERT(itemSize_B > 0);
    RTASSERT(itemSize_B <= fifo->itemSize_B);

    if (fifo->size < fifo->capacity) {
        RTByte* dst = &(fifo->buffer[fifo->head * fifo->itemSize_B]);
        RTMemcpy(dst, fifo->itemSize_B, item, itemSize_B);

        /* Increment head */
        fifo->head++;
        if (fifo->head >= fifo->capacity) {
            fifo->head = 0;
        }
        fifo->size++;
        pushed = RTTrue;
    }
    return pushed;
}


RTBool RTFifoPop(RTFifo* fifo, void* item, uint16_t itemSize_B)
{
    RTBool popped = RTFalse;

    RTASSERT(fifo != NULL);
    RTASSERT(fifo->buffer != NULL);
    RTASSERT(item != NULL);
    RTASSERT(itemSize_B > 0);
    RTASSERT(itemSize_B >= fifo->itemSize_B);

    if (fifo->size > 0) {
        const RTByte* src = &(fifo->buffer[fifo->tail * fifo->itemSize_B]);
        RTMemcpy(item, itemSize_B, src, fifo->itemSize_B);

        /* Increment tail */
        fifo->tail++;
        if (fifo->tail >= fifo->capacity) {
            fifo->tail = 0;
        }
        fifo->size--;
        popped = RTTrue;
    }
    return popped;
}
