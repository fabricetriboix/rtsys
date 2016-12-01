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

/* You should not include this file directly; include "rtfifo.h" instead. */

#ifndef RTFIFO_PRIV_h_
#define RTFIFO_PRIV_h_

#include "rtplf.h"



/*----------------+
 | Types & Macros |
 +----------------*/


/** Small FIFO structure */
struct RTSmallFifo {
    uint8_t head;       /**< Head of the FIFO */
    uint8_t tail;       /**< Tail of the FIFO */
    uint8_t size;       /**< Size of the FIFO, in items */
    uint8_t capacity;   /**< Capacity of the FIFO, in items */
    uint8_t itemSize_B; /**< Size of one item, in bytes */
    RTByte* buffer;     /**< Where to store the items */
};


/** Macro initialiser for a statically-allocated small FIFO */
#define RTPRIV_SMALL_FIFO_INIT(_buffer) \
    {                                   \
        0,                              \
        0,                              \
        0,                              \
        RTARRAYSIZE(_buffer),           \
        sizeof((_buffer)[0]),           \
        (RTByte*)(_buffer)              \
    }


/** Regular FIFO structure */
struct RTFifo {
    uint16_t head;       /**< Head of the FIFO */
    uint16_t tail;       /**< Tail of the FIFO */
    uint16_t size;       /**< Size of the FIFO, in items */
    uint16_t capacity;   /**< Capacity of the FIFO, in items */
    uint16_t itemSize_B; /**< Size of one item, in bytes */
    RTByte*  buffer;     /**< Where to store the items */
};


/** Macro initialiser for a statically-allocated regular FIFO */
#define RTPRIV_FIFO_INIT(_buffer)   \
    {                               \
        0,                          \
        0,                          \
        0,                          \
        RTARRAYSIZE(_buffer),       \
        sizeof((_buffer)[0]),       \
        (RTByte*)(_buffer)          \
    }



#endif /* RTFIFO_PRIV_h_ */
