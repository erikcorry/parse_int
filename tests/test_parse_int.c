#include <inttypes.h>
#include <stdio.h>

#include "../parse_int.h"

#define MAKE_FAIL(name, type) \
void fail_##name(const char* str) { \
  type answer = 0; \
  const char* end = parse_##name(&answer, str); \
  if (end) { \
    printf("***Expected to fail on '%s', passed\n", str); \
  } \
  if (answer) { \
    printf("***Expected unchanged result on '%s'\n", str); \
  } \
}

#define MAKE_FAIL_SIZE(name, type) \
void fail_##name(const char* str, int size) { \
  type answer = 0; \
  const char* end = parse_##name(&answer, str, size); \
  if (end) { \
    printf("***Expected to fail on '%s', passed\n", str); \
  } \
  if (answer) { \
    printf("***Expected unchanged result on '%s'\n", str); \
  } \
}

MAKE_FAIL(int, int)
MAKE_FAIL(unsigned, unsigned)
MAKE_FAIL(u64, uint64_t)
MAKE_FAIL(64, int64_t)
MAKE_FAIL_SIZE(all_n_64, int64_t)
#ifdef __SIZEOF_INT128__
MAKE_FAIL(u128, __uint128_t)
#endif

#define MAKE_TRY(name, type, format) \
void try_##name(type expected_answer, int expected_chars, const char* str) { \
  type answer = 0; \
  const char* end = parse_##name(&answer, str); \
  if (!end) { \
    printf("***Expected to parse '%s', failed\n", str); \
  } \
  if (answer != expected_answer) { \
    printf("***Expected answer to '%s' of %" format ", got %" format "\n", str, expected_answer, answer); \
  } \
  if (end - str != expected_chars) { \
    printf("***Expected to take %d characters of '%s', took %zd\n", expected_chars, str, end - str); \
  } \
}

#define MAKE_TRY_SIZE(name, type, format) \
void try_##name(type expected_answer, int size, const char* str) { \
  type answer = 0; \
  const char* end = parse_##name(&answer, str, size); \
  if (!end) { \
    printf("***Expected to parse '%s', failed\n", str); \
  } \
  if (answer != expected_answer) { \
    printf("***Expected answer to '%s' of %" format ", got %" format "\n", str, expected_answer, answer); \
  } \
  if (end - str != size) { \
    printf("***Expected to take %d characters of '%s', took %zd\n", size, str, end - str); \
  } \
}

#define MAKE_TRY_128(name, type) \
void try_##name(type expected_answer, int expected_chars, const char* str) { \
  type answer = 0; \
  const char* end = parse_##name(&answer, str); \
  if (!end) { \
    printf("***Expected to parse '%s', failed\n", str); \
  } \
  if (answer != expected_answer) { \
    printf("***Wrong answer to '%s'\n", str); \
  } \
  if (end - str != expected_chars) { \
    printf("***Expected to take %d characters of '%s', took %ld\n", expected_chars, str, end - str); \
  } \
}

MAKE_TRY(int, int, "d")
MAKE_TRY(unsigned, unsigned, "u")
MAKE_TRY(u64, uint64_t, PRIu64)
MAKE_TRY(64, int64_t, PRId64)
MAKE_TRY_SIZE(all_n_64, int64_t, PRId64)
#ifdef __SIZEOF_INT128__
MAKE_TRY_128(u128, __uint128_t)
#endif

int main() {
  try_int(123, 3, "123");
  try_int(321, 3, "321");
  try_int(0, 1, "0");
  try_int(2147483647, 10, "2147483647");
  try_int(2147483647, 10, "2147483647/////////////");
  try_int(2147483647, 10, "2147483647:::::::::::::");
  try_int(-2147483648, 11, "-2147483648");
  try_int(-2147483648, 11, "-2147483648/////////////");
  try_int(-2147483648, 11, "-2147483648:::::::::::::");
  try_int(123456789, 9, "123456789");
  fail_int("x");
  fail_int("2147483648");
  fail_int("9999999999");
  fail_int("2147483650");

  try_unsigned(2147483647, 10, "2147483647");
  try_unsigned(2147483647, 10, "2147483647/////////////");
  try_unsigned(2147483647, 10, "2147483647:::::::::::::");
  try_unsigned(4294967295, 10, "4294967295");
  try_unsigned(4294967295, 10, "4294967295/////////////");
  try_unsigned(4294967295, 10, "4294967295:::::::::::::");
  fail_unsigned("4294967296");
  fail_unsigned("4294967300");
  fail_unsigned("9999999999");

  const char *pi = "3.14159";
  try_int(3, 1, pi);
  try_int(14159, 5, pi + 2);

  try_int(42, 40, "0000000000000000000000000000000000000042");
  fail_int("0000000000000000100000000000000000000042");

  try_u64(18446744073709551615LLU, 20, "18446744073709551615");
  fail_u64("18446744073709551616");

  try_64(0, 1, "0");
  try_64(0, 2, "-0");
  try_64(1, 1, "1");
  try_64(-1, 2, "-1");
  try_64(12, 2, "12");
  try_64(-12, 3, "-12");
  try_64(-123, 4, "-123");
  try_64(123456789, 9, "123456789");
  try_64(1234567890, 10, "1234567890");
  try_64(12345678901L, 11, "12345678901");
  try_64(1073741823, 10, "1073741823");
  try_64(1073741824, 10, "1073741824");
  try_64(-1073741823, 11, "-1073741823");
  try_64(-1073741823LL - 1, 11, "-1073741824");
  try_64(-1073741825LL, 11, "-1073741825");
  try_64(999999999999999999LL, 18, "999999999999999999");
  try_64(-999999999999999999LL, 19, "-999999999999999999");
  fail_64("9999999999999999999");
  fail_64("-9999999999999999999");
  fail_64("9223372036854775808");
  fail_64("-9223372036854775809");

  try_64(9223372036854775807LL, 19, "9223372036854775807");
  try_64(-9223372036854775807LL - 1, 20, "-9223372036854775808");
  fail_64("foo");
  fail_64("--1");
  fail_all_n_64("1-1", 3);
  fail_64("-");
  fail_64("-");

  fail_64("a");
  fail_64("c");
  fail_64("g");
  fail_64("h");

  fail_64("_");
  fail_64("_123");
  try_64(1, 1, "1_123");
  fail_all_n_64("1_123", 5);
  try_64(1012, 4, "1012_");
  fail_all_n_64("1012_", 5);
  try_64(1012, 4, "1012_1");
  fail_all_n_64("1012_1", 6);

#ifdef __SIZEOF_INT128__
  try_u128(42, 37, "0000000000000000000000000000000000042");
  __uint128_t e = 1;
  e *= 1000000000000000000LLU;
  e *= 1000000000000000000LLU;
  e += 42;
  try_u128(e, 37, "1000000000000000000000000000000000042");
  e = 0;
  e--;
  try_u128(e, 39, "340282366920938463463374607431768211455");
  fail_u128("340282366920938463463374607431768211456");
  fail_u128("999999999999999999999999999999999999999");
#endif
}
