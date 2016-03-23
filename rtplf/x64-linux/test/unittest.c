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
#include "rttest.h"


static uint16_t mystrlen(const char* str)
{
    uint16_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}


static int8_t mystrcmp(const char* str1, const char* str2)
{
    uint16_t i;
    uint16_t n = mystrlen(str1);
    uint16_t tmp = mystrlen(str2);
    int8_t ret;

    /* Take the smallest string length */
    if (tmp < n) {
        n = tmp;
    }

    /* NB: `i <= n` is not a bug! We want to compare the final '\0' as well */
    ret = 0;
    for (i = 0; (i <= n) && (ret == 0); i++) {
        if (str1[i] > str2[i]) {
            ret = 1;
        } else if (str1[i] < str2[i]) {
            ret = -1;
        }
    }

    return ret;
}



RTT_GROUP_START(TestI32ToString, 0x00010001u, NULL, NULL)

RTT_TEST_START(rt32tostr_should_print_positive_i32)
{
    char buf[32];
    RTT_EXPECT(RT32ToString(1200300400, buf, sizeof(buf)) == 10);
    RTT_EXPECT(mystrcmp(buf, "1200300400") == 0);
}
RTT_TEST_END

RTT_TEST_START(rt32tostr_should_print_negative_i32)
{
    char buf[32];
    RTT_EXPECT(RT32ToString(-756123, buf, sizeof(buf)) == 7);
    RTT_EXPECT(mystrcmp(buf, "-756123") == 0);
}
RTT_TEST_END

RTT_TEST_START(rt32tostr_should_truncate_i32)
{
    char buf[32];
    uint16_t i;
    for (i = 0; i < sizeof(buf); i++) {
        buf[i] = 'a';
    }
    RTT_EXPECT(RT32ToString(-5400, buf, 5) == 4);
    RTT_EXPECT(mystrcmp(buf, "-540") == 0);
    RTT_EXPECT(buf[5] == 'a');
}
RTT_TEST_END

RTT_TEST_START(rt32tostr_should_accept_null_arg)
{
    RTT_EXPECT(RT32ToString(7, NULL, 10) == 0);
}
RTT_TEST_END

RTT_TEST_START(rt32tostr_should_accept_zero_size)
{
    char buf[32];
    RTT_EXPECT(RT32ToString(5, buf, 0) == 0);
}
RTT_TEST_END

RTT_GROUP_END(TestI32ToString,
        rt32tostr_should_print_positive_i32,
        rt32tostr_should_print_negative_i32,
        rt32tostr_should_truncate_i32,
        rt32tostr_should_accept_null_arg,
        rt32tostr_should_accept_zero_size)


RTT_GROUP_START(TestU32ToString, 0x00010002u, NULL, NULL)

RTT_TEST_START(rtu32tostr_should_print_in_base_10)
{
    char buf[32];
    RTT_EXPECT(RTU32ToString(37u, RTBASE_10, buf, sizeof(buf)) == 2);
    RTT_EXPECT(mystrcmp(buf, "37") == 0);
}
RTT_TEST_END

RTT_TEST_START(rtu32tostr_should_print_in_base_8)
{
    char buf[32];
    RTT_EXPECT(RTU32ToString(03673451u, RTBASE_8, buf, sizeof(buf)) == 7);
    RTT_EXPECT(mystrcmp(buf, "3673451") == 0);
}
RTT_TEST_END

RTT_TEST_START(rtu32tostr_should_print_in_base_16)
{
    char buf[32];
    RTT_EXPECT(RTU32ToString(0xdeadbeefU, RTBASE_16, buf, sizeof(buf)) == 8);
    RTT_EXPECT(mystrcmp(buf, "deadbeef") == 0);
}
RTT_TEST_END

RTT_TEST_START(rtu32tostr_should_print_in_base_2)
{
    char buf[34];
    const char* expected = "11001010111111101101111011001010";
                         /* ccccaaaaffffeeeeddddeeeeccccaaaa */

    RTT_EXPECT(RTU32ToString(0xcafedecaU, RTBASE_2, buf, sizeof(buf)) == 32);
    RTT_EXPECT(mystrcmp(buf, expected) == 0);
}
RTT_TEST_END

RTT_TEST_START(rtu32tostr_should_print_in_base_auto)
{
    char buf[32];
    RTT_EXPECT(RTU32ToString(1074893997, RTBASE_AUTO, buf, sizeof(buf)) == 10);
    RTT_EXPECT(mystrcmp(buf, "1074893997") == 0);
}
RTT_TEST_END

RTT_TEST_START(rtu32tostr_should_truncate)
{
    char buf[32];
    uint16_t i;
    for (i = 0; i < sizeof(buf); i++) {
        buf[i] = 'a';
    }
    RTT_EXPECT(RTU32ToString(1074893997, RTBASE_10, buf, 4) == 3);
    RTT_EXPECT(mystrcmp(buf, "107") == 0);
    RTT_EXPECT(buf[4] == 'a');
}
RTT_TEST_END

RTT_TEST_START(rtu32tostr_should_accept_null_arg)
{
    RTT_EXPECT(RTU32ToString(4, RTBASE_AUTO, NULL, 4) == 0);
}
RTT_TEST_END

RTT_TEST_START(rtu32tostr_should_accept_zero_arg)
{
    char buf[32];
    RTT_EXPECT(RTU32ToString(9, RTBASE_AUTO, buf, 0) == 0);
}
RTT_TEST_END

RTT_GROUP_END(TestU32ToString,
        rtu32tostr_should_print_in_base_10,
        rtu32tostr_should_print_in_base_8,
        rtu32tostr_should_print_in_base_16,
        rtu32tostr_should_print_in_base_2,
        rtu32tostr_should_print_in_base_auto,
        rtu32tostr_should_truncate,
        rtu32tostr_should_accept_null_arg,
        rtu32tostr_should_accept_zero_arg)


RTT_GROUP_START(TestStringTo32, 0x00010003u, NULL, NULL)

RTT_TEST_START(strto32_should_parse_positive_number)
{
    int32_t x;
    RTT_EXPECT(RTStringTo32("276000123", &x));
    RTT_EXPECT(x == 276000123);
}
RTT_TEST_END

RTT_TEST_START(strto32_should_parse_negative_number)
{
    int32_t x;
    RTT_EXPECT(RTStringTo32("-9000", &x));
    RTT_EXPECT(x == -9000);
}
RTT_TEST_END

RTT_TEST_START(strto32_should_parse_positive_and_ignore_extra)
{
    int32_t x;
    RTT_EXPECT(RTStringTo32("123abc", &x));
    RTT_EXPECT(x == 123);
}
RTT_TEST_END

RTT_TEST_START(strto32_should_parse_negative_and_ignore_extra)
{
    int32_t x;
    RTT_EXPECT(RTStringTo32("-123789XYZ", &x));
    RTT_EXPECT(x == -123789);
}
RTT_TEST_END

RTT_TEST_START(strto32_should_ignore_blanks)
{
    int32_t x;
    RTT_EXPECT(RTStringTo32("12 34", &x));
    RTT_EXPECT(x == 12);
}
RTT_TEST_END

RTT_TEST_START(strto32_should_not_parse_non_digits)
{
    int32_t x;
    RTT_EXPECT(!RTStringTo32("hello world", &x));
}
RTT_TEST_END

RTT_TEST_START(strto32_should_not_parse_empty_string)
{
    int32_t x;
    RTT_EXPECT(!RTStringTo32("", &x));
}
RTT_TEST_END

RTT_TEST_START(strto32_should_not_parse_null_string)
{
    int32_t x;
    RTT_EXPECT(!RTStringTo32(NULL, &x));
}
RTT_TEST_END

RTT_TEST_START(strto32_should_take_null_arg)
{
    RTT_EXPECT(!RTStringTo32("123", NULL));
}
RTT_TEST_END

RTT_GROUP_END(TestStringTo32,
        strto32_should_parse_positive_number,
        strto32_should_parse_negative_number,
        strto32_should_parse_positive_and_ignore_extra,
        strto32_should_parse_negative_and_ignore_extra,
        strto32_should_ignore_blanks,
        strto32_should_not_parse_non_digits,
        strto32_should_not_parse_empty_string,
        strto32_should_not_parse_null_string,
        strto32_should_take_null_arg)


RTT_GROUP_START(TestStringToU32, 0x00010004u, NULL, NULL)

RTT_TEST_START(strtou32_should_parse_base_10)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("3222377645", RTBASE_10, &x));
    RTT_EXPECT(x == 3222377645u);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_ignore_extra_in_base_10)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("123000456 78", RTBASE_10, &x));
    RTT_EXPECT(x == 123000456u);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_parse_base_16)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("123456", RTBASE_16, &x));
    RTT_EXPECT(x == 0x123456u);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_ignore_extra_in_base_16)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("abcdefghijklmnopqrstuvwxyz", RTBASE_16, &x));
    RTT_EXPECT(x == 0xABCDEFu);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_parse_base_8)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("123456", RTBASE_8, &x));
    RTT_EXPECT(x == 0123456u);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_ignore_extra_in_base_8)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("1234567890", RTBASE_8, &x));
    RTT_EXPECT(x = 01234567u);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_parse_base_2)
{
    const char* s = "11001010111111101101111011001010";
                  /* ccccaaaaffffeeeeddddeeeeccccaaaa */
    uint32_t x;
    RTT_EXPECT(RTStringToU32(s, RTBASE_2, &x));
    RTT_EXPECT(x == 0xCAFEDECAu);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_ignore_extra_in_base_2)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("0123", RTBASE_2, &x));
    RTT_EXPECT(x == 1u);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_detect_base_10)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("70100", RTBASE_AUTO, &x));
    RTT_EXPECT(x == 70100u);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_detect_base_10_with_extra)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("70200blablabla", RTBASE_AUTO, &x));
    RTT_EXPECT(x == 70200u);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_detect_base_16)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("0XDeadBeef", RTBASE_AUTO, &x));
    RTT_EXPECT(x == 0xDEADBEEFu);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_detect_base_16_with_extra)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("0x70200blablabla", RTBASE_AUTO, &x));
    RTT_EXPECT(x == 0x70200Bu);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_detect_base_8)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("0123", RTBASE_AUTO, &x));
    RTT_EXPECT(x == 0123u);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_detect_base_8_with_extra)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("01234567890123", RTBASE_AUTO, &x));
    RTT_EXPECT(x == 01234567u);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_detect_base_2)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("b10100101", RTBASE_AUTO, &x));
    RTT_EXPECT(x == 0xA5u);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_detect_base_2_with_extra)
{
    uint32_t x;
    RTT_EXPECT(RTStringToU32("B11223344", RTBASE_AUTO, &x));
    RTT_EXPECT(x == 3u);
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_not_parse_non_digits)
{
    uint32_t x;
    RTT_EXPECT(!RTStringToU32("abcd", RTBASE_AUTO, &x));
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_not_parse_empty_string)
{
    uint32_t x;
    RTT_EXPECT(!RTStringToU32("", RTBASE_AUTO, &x));
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_not_parse_null_string)
{
    uint32_t x;
    RTT_EXPECT(!RTStringToU32(NULL, RTBASE_AUTO, &x));
}
RTT_TEST_END

RTT_TEST_START(strtou32_should_take_null_arg)
{
    RTT_EXPECT(!RTStringToU32("123", RTBASE_AUTO, NULL));
}
RTT_TEST_END

RTT_GROUP_END(TestStringToU32,
        strtou32_should_parse_base_10,
        strtou32_should_ignore_extra_in_base_10,
        strtou32_should_parse_base_16,
        strtou32_should_ignore_extra_in_base_16,
        strtou32_should_parse_base_8,
        strtou32_should_ignore_extra_in_base_8,
        strtou32_should_parse_base_2,
        strtou32_should_ignore_extra_in_base_2,
        strtou32_should_detect_base_10,
        strtou32_should_detect_base_10_with_extra,
        strtou32_should_detect_base_16,
        strtou32_should_detect_base_16_with_extra,
        strtou32_should_detect_base_8,
        strtou32_should_detect_base_8_with_extra,
        strtou32_should_detect_base_2,
        strtou32_should_detect_base_2_with_extra,
        strtou32_should_not_parse_non_digits,
        strtou32_should_not_parse_empty_string,
        strtou32_should_not_parse_null_string,
        strtou32_should_take_null_arg)
