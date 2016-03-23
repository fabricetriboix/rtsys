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

/** Platform-dependent stuff
 *
 * \defgroup rtplf_x64_linux Platform x64-linux
 * \addtogroup rtplf_x64_linux
 * @{
 *
 * Types and definitions specific to the 'x64-linux' architecture. You are
 * responsible for making sure that the information contained in this file is
 * correct and matches your actual architecture.
 *
 * In particular, the `RTByte` type should be the small addressable unit of
 * memory. On most architecture, that would be an 8-bit integer. It could be
 * different on other architectures; for example, some Texas Instrument DSPs
 * have a byte size of 16-bit.
 *
 * All strings in this module are composed of 8-bit ASCII characters. This
 * module offers no support for unicode. All strings are null-terminated unless
 * stated otherwise, and have a length of 60,000 characters or less; non-respect
 * of the above will lead to undefined behaviour. Some functions might accept
 * NULL values for string arguments, please check each function description.
 *
 * Important note about error handling. The policy adopted by this software
 * regarding error conditions is that errors come in two types:
 *  - Errors that we can expect to happen
 *  - Unexpected error
 *
 * Examples of errors that we can expect to happen:
 *  - Network unreachable
 *  - Checksum failure on a network packet
 *  - Serial communication failure (I2C, SPI, RS232, etc.)
 *  - Corrupted ROM, corrupted mass storage, corrupted EEPROM
 *  - Timeout when communicating with a device or over the network
 *
 * Examples of unexpected errors:
 *  - The software tries to read or write non-existent memory (mad pointer)
 *  - The software accesses an array out of boundary
 *  - Function called with out-of-range arguments, and the function's
 *    specification mentions that it will panic in such circumstances
 *  - Stack overflow
 *  - Unaligned memory accesses
 *  - Watchdog timeout
 *  - An assertion fails (pre/post condition, assert)
 *
 * When an error that we can expect to happen does occur, the function that
 * detects the error should handle it gracefully and return an error code if
 * appropriate. The error condition should be handled either by the function
 * itself, or by one of its callers.
 *
 * When an unexpected error does occurs, the panic function is called. In
 * theory, the panic function should never be called for a perfect software. In
 * practice, you still need to implement something sensible for when that
 * happens, like immediately shuting down the system, or making it safe, or
 * maybe resetting the board or restarting the software. You should pretty much
 * treat such a situation as if a watchdog timeout occurred.
 *
 * There is only one external dependency: stdint.h, to provide with standard
 * types for fixed-size integers. If you don't want any dependency on the C
 * standard library, you can provide this header file yourself, just make sure
 * all the standard integer types are provided.
 */


#ifndef RTPLF_X64_LINUX_h_
#define RTPLF_X64_LINUX_h_

#include <stdint.h>


/*----------------+
 | Panic function |
 +----------------*/


/** Panic function
 *
 * This function is called in case of unexpected problems. Please note that
 * invalid arguments to certain functions are sometimes considered unexpected
 * problems.
 *
 * Please refer to the introductory blurb at the beginning of this module for
 * more information.
 *
 * \return This function does not return
 */
void RTPanic(void);



/*--------+
 | Macros |
 +--------*/


/** NULL pointer */
#ifndef NULL
#define NULL ((void*)0)
#endif


/** Hard limit on string length
 *
 * This applies to any null-terminated string and includes the terminating null
 * character. Any operation on a string longer than that will result in an
 * assert.
 */
#define RTSTRMAX 50000u


/** Assert macro
 *
 * This macro is used to add asserts in the code. It will call `RTPanic()` if
 * the condition is not met.
 */
#define RTASSERT(_cond) \
    do { \
        if (!(_cond)) { \
            RTPanic(); \
        } \
    } while (0)


/** Get the size of a statically allocated array */
#define RTARRAYSIZE(_array) (sizeof(_array) / sizeof((_array)[0]))



/*-------+
 | Types |
 +-------*/


/** Byte: smallest addressable datum
 *
 * **IMPORTANT**: Please make sure this matches your architecture!
 */
typedef uint8_t RTByte;


/** Boolean */
typedef enum
{
    RTFalse = 0,
    RTTrue = 1
} RTBool;


/** Numerical bases */
typedef enum
{
    RTBASE_AUTO,
    RTBASE_2,
    RTBASE_8,
    RTBASE_10,
    RTBASE_16,
    RTBASE_COUNT
} RTBase;



/*------------------------------+
 | Public function declarations |
 +------------------------------*/


/** Get the current time, in ticks
 *
 * Ticks can be whatever your hardware provides, provided that the tick
 * frequency is stable.
 *
 * \return Current timestamp, in ticks
 */
uint32_t RTNow_tick(void);


/** Get the tick frequency
 *
 * \return Tick frequency, in Hz
 */
uint32_t RTTickFrequency_Hz(void);


/** Get the current time, in us
 *
 * This function might be less accurate than `RTNow_tick()`.
 *
 * \return Current timestamp, in us
 */
uint32_t RTNow_us(void);


/** Converts a 32-bit signed integer into a string
 *
 * If the provided buffer is too small, the string is truncated. In any case,
 * the string will be null-terminated. The integer will be printed in base 10.
 *
 * This function will assert if one (or more) of its argument is invalid.
 *
 * \param x      [in]  The integer to convert
 * \param buffer [out] Where to write the string. This argument is allowed to be
 *                     NULL, in which case no action is taken and this function
 *                     returns 0.
 * \param size   [in]  Size of the above buffer, in char. This argument is
 *                     allowed to be 0, in which case no action is taken and
 *                     this function returns 0.
 *
 * \return The number of characters written, excluding the terminating null
 *         character
 */
uint16_t RT32ToString(int32_t x, char* buffer, uint16_t size);


/** Converts a 32-bit unsigned integer into a string
 *
 * If the provided buffer is too small, the string is truncated. In any case,
 * the string will be null-terminated.
 *
 * This function will assert if one (or more) of its argument is invalid.
 *
 * \param x      [in]  The integer to convert
 * \param base   [in]  The numerical base to use; if set to `RTBASE_AUTO`, base
 *                     10 will be used
 * \param buffer [out] Where to write the string. This argument is allowed to be
 *                     NULL, in which case no action is taken and this function
 *                     returns 0;
 * \param size   [in]  Size of the above buffer, in char. This argument is
 *                     allowed to be 0, in which case no action is taken and
 *                     this function returns 0;
 *
 * \return The number of characters written, excluding the terminating null
 *         character
 */
uint16_t RTU32ToString(uint32_t x, RTBase base, char* buffer, uint16_t size);


/** Converts a string into a 32-bit signed integer
 *
 * This function should attempt to parse as many characters as possible. The
 * entire string might not be a number.
 * For example: "-123abc" should parse the number -123 and return `RTTrue`.
 *
 * This function will assert if one (or more) of its argument is invalid.
 *
 * \param str [in]  The string to parse. This argument is allowed to be NULL, in
 *                  which case no action is taken and this function returns
 *                  `RTFalse`.
 * \param x   [out] The parsed integer. This argument is allowed to be NULL, in
 *                  which case no action is taken and this function returns
 *                  `RTTrue`.
 *
 * \return `RTTrue` if integer successfully parsed, `RTFalse` otherwise
 */
RTBool RTStringTo32(const char* str, int32_t* x);


/** Converts a string into a 32-bit unsigned integer
 *
 * This function should attempt to parse as many characters as possible. The
 * entire string might not be a number.
 * For example: "123abc" should parse the number 123 and return `RTTrue`.
 *
 * This function will assert if one (or more) of its argument is invalid.
 *
 * \param str  [in]  The string to parse. This argument is allowed to be NULL,
 *                   in which case no action is taken and this function returns
 *                   `RTFalse`.
 * \param base [in]  The numerical base to use. If set to `RTBASE_AUTO`, the
 *                   base to use is inferred from the first few characters of
 *                   the string:
 *                     - If the string starts with "0x" or "0X", base 16 is used
 *                     - If the string starts with "0" followed by at least one
 *                       digit, base 8 is used
 *                     - If the string starts with 'b' or 'B', base 2 is used
 *                     - In any other case, base 10 is used
 * \param x    [out] The parsed integer. This argument is allowed to be NULL, in
 *                   which case no action is taken and this function returns
 *                   `RTTrue`.
 *
 * \return `RTTrue` if integer successfully parsed, `RTFalse` otherwise
 */
RTBool RTStringToU32(const char* str, RTBase base, uint32_t* x);


/** Fast memory copy
 *
 * Copy the `src` memory area to the `dst` memory area. The memory areas must
 * not overlap (if they do, the behaviour of this function is undefined).
 * The number of bytes copied is the lower of `srcSize_B` or `dstSize_B`.
 *
 * \param dst       [out] Where to copy the data. This argument is allowed to be
 *                        NULL, in which case no action is taken.
 * \param dstSize_B [in]  Size of the `dst` buffer, in bytes. This argument is
 *                        allowed to be 0, in which case no action is taken.
 * \param src       [in]  Data source. This argument is allowed to be NULL, in
 *                        which case no action is taken.
 * \param srcSize_B [in]  Size of the `src` buffer, in bytes. This argument is
 *                        allowed to be 0, in which case no action is taken.
 */
void RTMemcpy(RTByte* dst, uint16_t dstSize_B,
		const RTByte* src, uint16_t srcSize_B);


/** Compute the length of a string
 *
 * The length of a string is the number of characters until the null character.
 * For example: for 'f', 'o', 'o', '\0', the function must return 3.
 *
 * \param str [in] The string to query. This argument is allowed to be NULL, in
 *                 which case this function returns 0.
 *
 * \return The length of the string, excluding the null terminating character
 */
uint16_t RTStrlen(const char* str);


/** Copy a string
 *
 * The string copy will always be null-terminated, even if truncated.
 *
 * \param dst     [out] Where to write the copied string. This argument is
 *                      allowed to be NULL, in which case no action is taken,
 *                      and this function returns `RTTrue` if `src` is also
 *                      NULL, and `RTFalse` otherwise.
 * \param dstSize [in]  Size of the `dst` buffer, in char. This argument is
 *                      allowed to be 0, in which case this function behaves
 *                      like `dst` was set to NULL. At most `dstSize` characters
 *                      are copied into `dst`, including the terminating null
 *                      character. The `src` string will only be partially
 *                      copied if it is too long.
 * \param src     [in]  The string to copy. This argument is allowed to be NULL,
 *                      in which case `dst` is set to the empty string ""
 *                      (provided that it is itself not NULL and that `dstSize`
 *                      is > 0), and this function returns `RTTrue`.
 *
 * \return `RTTrue` if the string has been entirely copied, `RTFalse` if it has
 *         been truncated
 */
RTBool RTStrncpy(char* dst, uint16_t dstSize, const char* src);


/** Compare 2 strings
 *
 * \param str1 [in] The first string to compare. This argument is allowed to be
 *                  NULL, in which case no action is taken and this function
 *                  returns 0 if `str2` is also NULL, and -1 otherwise.
 * \param str2 [in] The second string to compare. This argument is allowed to be
 *                  NULL, in which case no action is taken and this function
 *                  returns 0 if `str` is also NULL, and 1 otherwise.
 *
 * \return -1 if `str1` < `str2`, 0 if `str1` == `str2`, +1 if `str1` > `str2`
 */
int8_t RTStrcmp(const char* str1, const char* str2);


/** Compare the first `n` characters of 2 strings
 *
 * \param str1 [in] The first string to compare. This argument is allowed to be
 *                  NULL, in which case no action is taken and this function
 *                  returns 0 if `str2` is also NULL, and -1 otherwise.
 * \param str2 [in] The second string to compare. This argument is allowed to be
 *                  NULL, in which case no action is taken and this function
 *                  returns 0 if `str` is also NULL, and 1 otherwise.
 * \param size [in] The number of characters to compare, not including the
 *                  terminating null character. This argument is allowed to be
 *                  0, in which case no action is taken and this function
 *                  returns 0.
 *
 * \return -1 if `str1` < `str2`, 0 if `str1` == `str2`, +1 if `str1` > `str2`
 */
int8_t RTStrncmp(const char* str1, const char* str2, uint16_t size);


#endif /* RTPLF_X64_LINUX_h_ */
/* @} */
