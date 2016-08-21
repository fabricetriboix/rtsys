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

/** Implementation of platform-dependent features for x64-linux
 *
 * NB: This file will not comply with MISRA C 2012, because you wouldn't be
 * using Linux is such a case anyway.
 */

#include "rtplf.h"
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>



/*-------------------------------+
 | Private function declarations |
 +-------------------------------*/


/** Print a 32-bit unsigned integer in base 2
 *
 * No prefix is printed.
 *
 * @param x      [in]  The 32-bit unsigned integer to print
 * @param buffer [out] Where to print. This argument must not be NULL. The
 *                     string will always be null-terminated.
 * @param size   [in]  Size of the above buffer, in char; must be > 0. If the
 *                     size is not enough, the string will be truncated.
 *
 * @return The number of characters written, not including the terminating null
 *         character; if the string has been truncated, the return value will
 *         be the number of characters that would have been written if the
 *         buffer was big enough.
 */
static int rtplfPrintBase2(uint32_t x, char* buffer, uint16_t size);


/** Scan a 32-bit unsigned integer in base 2
 *
 * @param str [in]  The string to parse; must not be NULL
 * @param x   [out] The parsed integer
 *
 * @return `RTTrue` if successfully parsed, `RTFalse` otherwise
 */
static RTBool rtplfScanBase2(const char* str, uint32_t* x);



/*---------------------------------+
 | Public function implementations |
 +---------------------------------*/


void RTPanic(void)
{
    abort();
}


uint32_t RTNow_tick(void)
{
    struct timeval now;
    int ret = gettimeofday(&now, NULL);
    RTASSERT(0 == ret);
    return (((uint32_t)now.tv_sec) * 1000000u) + (uint32_t)now.tv_usec;
}


uint32_t RTTickFrequency_Hz(void)
{
    return 1000000u;
}


uint32_t RTNow_us(void)
{
    return RTNow_tick();
}


uint16_t RT32ToString(int32_t x, char* buffer, uint16_t size)
{
    uint16_t nchars = 0;

    if ((buffer != NULL) && (size > 0)) {
        char tmp[16];
        nchars = (uint16_t)sprintf(tmp, "%ld", (long)x);
        if (nchars >= size) {
            nchars = size - 1;
        }
        RTStrncpy(buffer, size, tmp);
    }
    return nchars;
}


uint16_t RTU32ToString(uint32_t x, RTBase base, char* buffer, uint16_t size)
{
    uint16_t nchars = 0;
    char tmp[64];

    if ((buffer != NULL) && (size > 0)) {
        int n;
        switch (base)
        {
        case RTBASE_2 :
            n = rtplfPrintBase2(x, tmp, size);
            break;

        case RTBASE_8 :
            n = sprintf(tmp, "%lo", (unsigned long)x);
            break;

        case RTBASE_16 :
            n = sprintf(tmp, "%lx", (unsigned long)x);
            break;

        case RTBASE_AUTO :
        case RTBASE_10 :
        default :
            n = sprintf(tmp, "%lu", (unsigned long)x);
            break;
        }
        RTASSERT(n >= 0);
        if (n >= (int)size) {
            n = (int)size - 1;
        }
        nchars = (uint16_t)n;
        RTStrncpy(buffer, size, tmp);
    }
    return nchars;
}


RTBool RTStringTo32(const char* str, int32_t* x)
{
    RTBool parsed = RTFalse;

    if ((RTStrlen(str) > 0) && (x != NULL)) {
        long tmp;
        int n = sscanf(str, "%ld", &tmp);
        if (n == 1) {
            *x = tmp;
            parsed = RTTrue;
        }
    }
    return parsed;
}


RTBool RTStringToU32(const char* str, RTBase base, uint32_t* x)
{
    RTBool parsed = RTFalse;

    if ((RTStrlen(str) > 0) && (x != NULL)) {
        int n;
        unsigned long tmp;
        const char* start = str;
        if (base == RTBASE_AUTO) {
            /* We have been asked to guess the base */
            if (str[0] == '0') {
                if ((str[1] == 'x') || (str[1] == 'X')) {
                    base = RTBASE_16;
                    start += 2; /* skip starting "0x" */
                } else {
                    base = RTBASE_8;
                    start++; /* skip starting "0" */
                }
            } else if ((str[0] == 'b') || (str[0] == 'B')) {
                base = RTBASE_2;
                start++; /* skip the starting "b" */
            } else {
                base = RTBASE_10;
            }
        }

        switch (base)
        {
        case RTBASE_2 :
            n = rtplfScanBase2(start, x);
            if (n == 1) {
                parsed = RTTrue;
            }
            break;

        case RTBASE_8 :
            n = sscanf(start, "%lo", &tmp);
            if (n == 1) {
                *x = tmp;
                parsed = RTTrue;
            }
            break;

        case RTBASE_16 :
            n = sscanf(start, "%lx", &tmp);
            if (n == 1) {
                *x = tmp;
                parsed = RTTrue;
            }
            break;

        case RTBASE_10 :
        default :
            n = sscanf(start, "%lu", &tmp);
            if (n == 1) {
                *x = tmp;
                parsed = RTTrue;
            }
            break;
        }
    }
    return parsed;
}


void RTMemcpy(RTByte* dst, uint16_t dstSize_B,
        const RTByte* src, uint16_t srcSize_B)
{
    if ((dst != NULL) && (dstSize_B > 0) && (src != NULL) && (srcSize_B > 0)) {
        uint16_t size_B;
        if (dstSize_B < srcSize_B) {
            size_B = dstSize_B;
        } else {
            size_B = srcSize_B;
        }
        memcpy(dst, src, size_B);
    }
}


uint16_t RTStrlen(const char* str)
{
    uint16_t len = 0;
    if (str != NULL) {
        while (str[len] != '\0') {
            len++;
            RTASSERT(len < RTSTRMAX);
        }
    }
    return len;
}


RTBool RTStrncpy(char* dst, uint16_t dstSize, const char* src)
{
    RTBool truncated = RTTrue;

    RTASSERT(dstSize <= RTSTRMAX);

    if ((dst == NULL) || (dstSize <= 0)) {
        if (src == NULL) {
            truncated = RTFalse;
        }
    } else if (src == NULL) {
        dst[0] = '\0';
        truncated = RTTrue;
    } else {
        truncated = RTStrlen(src) >= dstSize;
        strncpy(dst, src, dstSize);
        if (truncated) {
            dst[dstSize - 1] = '\0';
        }
    }
    return truncated;
}


int8_t RTStrcmp(const char* str1, const char* str2)
{
    int8_t ret;

    if (str1 == NULL) {
        if (str2 == NULL) {
            ret = 0;
        } else {
            ret = -1;
        }
    } else if (str2 == NULL) {
        ret = 1;
    } else {
        uint16_t i;
        ret = 0;
        for (i = 0; ret == 0; i++) {
            RTASSERT(i <= RTSTRMAX);
            if (str1[i] < str2[i]) {
                ret = -1;
            }
            if (str1[i] > str2[i]) {
                ret = 1;
            }
        }
    }
    return ret;
}


int8_t RTStrncmp(const char* str1, const char* str2, uint16_t size)
{
    int8_t ret = 0;

    RTASSERT(size <= RTSTRMAX);

    if (size > 0) {
        if (str1 == NULL) {
            if (str2 == NULL) {
                ret = 0;
            } else {
                ret = -1;
            }
        } else if (str2 == NULL) {
            ret = 1;
        } else {
            uint16_t i;
            ret = 0;
            for (i = 0; ret == 0; i++) {
                RTASSERT(i <= RTSTRMAX);
                if (str1[i] < str2[i]) {
                    ret = -1;
                }
                if (str1[i] > str2[i]) {
                    ret = 1;
                }
            }
        }
    }
    return ret;
}



/*----------------------------------+
 | Private function implementations |
 +----------------------------------*/


static int rtplfPrintBase2(uint32_t x, char* buffer, uint16_t size)
{
    int i;
    int n = 0;   /* Number of characters actually written into 'str' (excluding
                  * terminating null byte) */
    int ret = 0; /* Number of characters that would have been written if `str`
                  * was long enough (exluding terminating null byte) */

    RTASSERT(buffer != NULL);
    RTASSERT(size > 0);

    for (i = 0; i < 32; i++) {
        char c;
        if (x & 0x80000000u) {
            c = '1';
        } else {
            c = '0';
        }

        /* NB: Leave some space for the terminating null char */
        if (ret < ((int)size - 1)) {
            buffer[n] = c;
            n++;
        }

        ret++;
        x <<= 1;
    }

    buffer[n] = '\0';
    return ret;
}


static RTBool rtplfScanBase2(const char* str, uint32_t* x)
{
    RTBool parsed = RTFalse;
    uint16_t len = RTStrlen(str);

    RTASSERT(str != NULL);
    RTASSERT(x !=  NULL);

    if (len > 0) {
        uint32_t tmp = 0;
        uint16_t i;
        RTBool parsing = RTTrue;
        RTBool parsedOneChar = RTFalse;

        if (len > 32u) {
            len = 32u; /* Parse only the first 32 characters */
        }

        for (i = 0; (i < len) && parsing; i++) {
            switch (str[i])
            {
            case '0' :
                if (parsing) {
                    tmp <<= 1;
                }
                parsedOneChar = RTTrue;
                break;

            case '1' :
                if (parsing) {
                    tmp <<= 1;
                    tmp |= 1;
                }
                parsedOneChar = RTTrue;
                break;

            default :
                parsing = RTFalse;
                break;
            }
        }

        if (parsing || parsedOneChar) {
            *x = tmp;
            parsed = RTTrue;
        }
    }
    return parsed;
}
