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

/** FIFOs
 *
 * @defgroup rtfifo FIFOs
 * @addtogroup rtfifo
 * @{
 *
 * FIFOs.
 */

#ifndef RTFIFO_h_
#define RTFIFO_h_

#include "rtplf.h"
#include "rtfifo_priv.h"



/*----------------+
 | Types & Macros |
 +----------------*/


/** "Opaque" type that represents a small FIFO
 *
 * This FIFO can take up to 255 items. Items must be of the same size, which can
 * be up to 255 bytes.
 *
 * *Important note*: Never access the structure directly! Always use the FIFO
 * functions.
 */
typedef struct RTPrivSmallFifo RTSmallFifo;


/** Macro initialiser for a statically-allocated small FIFO
 *
 * This macro can be used to initialise a small FIFO when the underlying buffer
 * has been previously *statically* declared as an array.
 *
 * The FIFO will then take ownership of the `_buffer`, which should then not be
 * accessed by anything else.
 *
 * For example:
 *   typedef struct { ... } MyStruct;
 *   static MyStruct gMyBuffer[32];
 *   static RTSmallFifo gMyFifo = RT_SMALL_FIFO_INIT(gMyBuffer);
 *
 * The above example is the recommended way to declare and initialise a FIFO.
 */
#define RT_SMALL_FIFO_INIT(_buffer) RTPRIV_SMALL_FIFO_INIT(_buffer)


/** "Opaque" type that represents a regular FIFO
 *
 * This FIFO can take up to 65,535 items. Items must be of the same size, which
 * can be up to 65,535 bytes.
 *
 * *Important note*: Never access the structure directly! Always use the FIFO
 * functions.
 */
typedef struct RTPrivFifo RTFifo;


/** Macro initialiser for a statically-allocated regular FIFO
 *
 * This macro can be used to initialise a regular FIFO when the underlying
 * buffer has been previously *statically* declared as an array.
 *
 * The FIFO will then take ownership of the `_buffer`, which should then not be
 * accessed by anything else.
 *
 * For example:
 *   typedef struct { ... } MyStruct;
 *   static MyStruct gMyBuffer[32];
 *   static RTFifo gMyFifo = RT_FIFO_INIT(gMyBuffer);
 *
 * The above example is the recommended way to declare and initialise a FIFO.
 */
#define RT_FIFO_INIT(_buffer) RTPRIV_FIFO_INIT(_buffer)



/*------------------------------+
 | Public function declarations |
 +------------------------------*/


/** Dynamically initialise a small FIFO
 *
 * This function can be used to initialise a small FIFO dynamically.
 *
 * **DO NOT** call this function on a FIFO that has been already initialised
 * with `RT_SMALL_FIFO_INIT()`!
 *
 * @param fifo       [in,out] FIFO structure to initialise; must not be NULL.
 * @param capacity   [in]     FIFO capacity, in number of items; must be > 0.
 * @param itemSize_B [in]     Size of a single item in the FIFO, in bytes; must
 *                            be > 0.
 * @param buffer     [in]     Where the FIFO items should be stored. `buffer`
 *                            must not be NULL and must point to a memory area
 *                            at least `capacity` * `itemSize_B` in size (in
 *                            bytes).
 *
 * The FIFO will then take ownership of the `buffer`, which should then not be
 * accessed by anything else thereafter.
 *
 * @return Nothing
 */
void RTSmallFifoInit(RTSmallFifo* fifo, uint8_t capacity,
        uint8_t itemSize_B, RTByte* buffer);


/** Get the size of a small FIFO
 *
 * @param fifo [in] FIFO to query; must not be NULL.
 *
 * @return The number of items currently stored in the FIFO
 */
uint8_t RTSmallFifoSize(const RTSmallFifo* fifo);


/** Get the capacity of a small FIFO
 *
 * @param fifo [in] FIFO to query; must not be NULL.
 *
 * @return The maximum number of items the FIFO can hold
 */
uint8_t RTSmallFifoCapacity(const RTSmallFifo* fifo);


/** Test if a small FIFO is empty
 *
 * @param fifo [in] FIFO to query; must not be NULL.
 *
 * @return `RTTrue` if the FIFO is empty, `RTFalse` if not
 */
RTBool RTSmallFifoIsEmpty(const RTSmallFifo* fifo);


/** Test if a small FIFO is full
 *
 * @param fifo [in] FIFO to query; must not be NULL.
 *
 * @return `RTTrue` if the FIFO is full, `RTFalse` if not
 */
RTBool RTSmallFifoIsFull(const RTSmallFifo* fifo);


/** Push an item into a small FIFO
 *
 * @param fifo       [in,out] FIFO where to push the item; must not be NULL.
 * @param item       [in]     The item to push; must not be NULL. The item
 *                            itself will be copied, so you retain the ownership
 *                            of the `item`.
 * @param itemSize_B [in]     The size of the `item`, in bytes. `itemSize_B`
 *                            must be > 0 and <= the size of FIFO items (as set
 *                            when the FIFO is initialised).
 *
 * @return `RTTrue` if success, `RTFalse` if the FIFO is full
 */
RTBool RTSmallFifoPush(RTSmallFifo* fifo, const void* item, uint8_t itemSize_B);


/** Pop an item from a small FIFO
 *
 * @param fifo       [in,out] FIFO from where to pop the item; must not be NULL.
 * @param item       [out]    Where to write the popped item; must not be NULL.
 * @param itemSize_B [in]     Size of the `item` buffer, in bytes. `itemSize_B`
 *                            must be >= the size of the FIFO items (as set
 *                            when the FIFO is initialised).
 *
 * @return `RTTrue` if success, `RTFalse` if FIFO is empty
 */
RTBool RTSmallFifoPop(RTSmallFifo* fifo, void* item, uint8_t itemSize_B);



/** Dynamically initialise a regular FIFO
 *
 * This function can be used to initialise a regular FIFO dynamically.
 *
 * **DO NOT** call this function on a FIFO that has been already initialised
 * with `RT_FIFO_INIT()`.
 *
 * @param fifo       [in,out] FIFO structure to initialise; must not be NULL.
 * @param capacity   [in]     FIFO capacity, in number of items; must be > 0.
 * @param itemSize_B [in]     Size of a single item in the FIFO, in bytes; must
 *                            be > 0.
 * @param buffer     [in]     Where the FIFO items should be stored. `buffer`
 *                            must not be NULL and must point to a memory area
 *                            at least `capacity` * `itemSize_B` in size (in
 *                            bytes).
 *
 * The FIFO will then take ownership of the `buffer`, which should then not be
 * accessed by anything else thereafter.
 *
 * @return Nothing
 */
void RTFifoInit(RTFifo* fifo, uint16_t capacity,
        uint16_t itemSize_B, RTByte* buffer);


/** Get the size of a regular FIFO
 *
 * @param fifo [in] FIFO to query; must not be NULL.
 *
 * @return The number of items currently stored in the FIFO
 */
uint16_t RTFifoSize(const RTFifo* fifo);


/** Get the capacity of a regular FIFO
 *
 * @param fifo [in] FIFO to query; must not be NULL.
 *
 * @return The maximum number of items the FIFO can hold
 */
uint16_t RTFifoCapacity(const RTFifo* fifo);


/** Test if a regular FIFO is empty
 *
 * @param fifo [in] FIFO to query; must not be NULL.
 *
 * @return `RTTrue` if the FIFO is empty, `RTFalse` if not
 */
RTBool RTFifoIsEmpty(const RTFifo* fifo);


/** Test if a regular FIFO is full
 *
 * @param fifo [in] FIFO to query; must not be NULL.
 *
 * @return `RTTrue` if the FIFO is full, `RTFalse` if not
 */
RTBool RTFifoIsFull(const RTFifo* fifo);


/** Push an item into a regular FIFO
 *
 * @param fifo       [in,out] FIFO where to push the item; must not be NULL.
 * @param item       [in]     The item to push; must not be NULL. The item
 *                            itself will be copied, so you retain the ownership
 *                            of the `item`.
 * @param itemSize_B [in]     The size of the `item`, in bytes. `itemSize_B`
 *                            must be > 0 and <= the size of FIFO items (as set
 *                            when the FIFO is initialised).
 *
 * @return `RTTrue` if success, `RTFalse` if the FIFO is full
 */
RTBool RTFifoPush(RTFifo* fifo, const void* item, uint16_t itemSize_B);


/** Pop an item from a regular FIFO
 *
 * @param fifo       [in,out] FIFO from where to pop the item; must not be NULL.
 * @param item       [out]    Where to write the popped item; must not be NULL.
 * @param itemSize_B [in]     Size of the `item` buffer, in bytes. `itemSize_B`
 *                            must be >= the size of the FIFO items (as set
 *                            when the FIFO is initialised).
 *
 * @return `RTTrue` if success, `RTFalse` if FIFO is empty
 */
RTBool RTFifoPop(RTFifo* fifo, void* item, uint16_t itemSize_B);


#endif /* RTFIFO_h_ */
/* @} */
