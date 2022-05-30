// Copyright 2022 Erik Corry.  See the LICENSE file, the MIT license.

#pragma once

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__SSSE3__) || defined(__SSE2_MATH__)
#include <emmintrin.h>
#include <stdalign.h>
#include <tmmintrin.h>
#endif

// Various routines for parsing integers.  The integers can be assumed to be
// null terminated or terminated with non-integer characters.  Alternatively
// you can pass an explicit string size.

// For unsigned methods the result argument is a read-write argument - if it
// already contains a non-zero value then function works as if that value was
// textually prepended to the integer being parsed.  If you start with a non-
// zero value there is no check for overflow caused by too many digits, only
// for a value that is too high despite having an OK number of digits.  You
// can get around this by setting the size argument to a value that prevents too
// many digits.

// For signed types an initial '-' is allowed.

// Return null if no integer was found or for out-of-range.  Otherwise return a
// pointer to the end of the number.

// Replace `int` with any of `unsigned`, `long`, `unsigned long`, `long_long`,
// `unsigned_long_long`,`int32`, `uint32`, `int64`, `uint64`, `int128`, or
// `uint128` (the last two only on 64 bit platforms, currently).

// Parses up to the first char that is not integer or to null char.
const inline char *parse_int(int *result, const char *p);

// Takes a null terminated string and expects whole string to be a number.
// Returns null if there is trailing junk after the integer.
const inline char *parse_all_int(int *result, const char *p);

// Takes an additional size and stops at whatever comes first: a char
// that is not an integer or the end of the string.
const inline char *parse_n_int(int *result, const char *p, size_t size);

// Takes an additional length pointer and returns null if there is trailing junk
// after the string.
const inline char *parse_all_n_int(int *result, const char *start, size_t size);

// Takes an additional length pointer.  Does not check the input for non-
// numeric characters or overflow.  The assumption is that this was checked
// previously.
const inline char *parse_nocheck_int(int *result, const char *start, size_t size);

// Takes an additional length pointer.  Does not check the input for overflow.
// The assumption is that this was checked previously, perhaps by choosing an
// integer type that is certain to be big enough.  Both this and
// `parse_overflow_int` will ignore overflow, the difference is that this one
// takes a size argument.
const inline char *parse_nooverflow_int(int *result, const char *start, size_t size);

// Does not check the input for overflow.  The assumption is that this will be
// checked afterwards by looking at the number of digits.  Both this and
// `parse_nooverflow_int` will ignore overflow, the difference is that this one
// does not take a size argument.
const inline char *parse_overflow_int(int *result, const char *start);

// Takes an additional length pointer.  Does not check the input for non-
// numeric characters.  The assumption is that this was checked previously.
const inline char *parse_novalidate_int(int *result, const char *start, size_t size);

// The 16-bytes-at-a-time mode uses the full 128 bit width of an SSE2 register.
// Unfortunately it's hard to do shift operations over 128 bits and it's slower
// than the non-SSE 64 bit mode.

#if defined(__SSSE3__) && defined (SIXTEEN_BYTES_AT_A_TIME)

#define _WORD uint64_t
#define _CHUNK __m128i
#define _CHUNK_SIZE 16
#define _REPEAT(value) _mm_set1_epi8(value)
#define _REPEAT_16(value) _mm_set1_epi16(value)
#define _REPEAT_32(value) _mm_set1_epi32(value)
#define _LOAD_FROM(p) (*((_CHUNK*)(p)) ^ _REPEAT('0'))

#define _DEFINE_STATIC_CONSTS do { } while(false)

#define _ZERO_FIRST_N_BYTES(value, n) ((value) & _GET_MASK(n))
// The vector has been xored with 0x30 so that digits are in the range 0-9.
// We do a saturating bytewise subtract so that all digits are 0, and all other
// characters are non-zero, then compare with all zeros.  We then extract all the
// top bits to a 16 bit regular integer.
#define _MAP_OF_NON_DIGITS(value) (_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_subs_epu8((value), _REPEAT(9)), _REPEAT(0))))
#define _MAP_TYPE int
#define _MAP_HAS_NON_DIGITS(value) ((value) != 0xffff)
// We are looking for the index of the least significant 0-bit.  The intrinsic
// finds the least significant 1-bit, so we add 1, which flips the string of 1-bits
// at the end.  The intrinsic returns 1 more than the index, so we have to subtract
// one.
#define _DIGITS_IN_MAP(value) (__builtin_ffs((value) + 1) - 1)
// Why is this so hard?
#define _SHIFT_LEFT_N_BYTES(value, n) (_mm_shuffle_epi8((value), _GET_SHUFFLE(n)))

__m128i _GET_MASK(_WORD i);
inline __m128i _GET_MASK(_WORD i) {
  static __m128i masks[17] = {
      {(int64_t)0xffffffffffffffffL, (int64_t)0xffffffffffffffffL},
      {(int64_t)0xffffffffffffff00L, (int64_t)0xffffffffffffffffL},
      {(int64_t)0xffffffffffff0000L, (int64_t)0xffffffffffffffffL},
      {(int64_t)0xffffffffff000000L, (int64_t)0xffffffffffffffffL},
      {(int64_t)0xffffffff00000000L, (int64_t)0xffffffffffffffffL},
      {(int64_t)0xffffff0000000000L, (int64_t)0xffffffffffffffffL},
      {(int64_t)0xffff000000000000L, (int64_t)0xffffffffffffffffL},
      {(int64_t)0xff00000000000000L, (int64_t)0xffffffffffffffffL},
      {(int64_t)0x0000000000000000L, (int64_t)0xffffffffffffffffL},
      {(int64_t)0x0000000000000000L, (int64_t)0xffffffffffffff00L},
      {(int64_t)0x0000000000000000L, (int64_t)0xffffffffffff0000L},
      {(int64_t)0x0000000000000000L, (int64_t)0xffffffffff000000L},
      {(int64_t)0x0000000000000000L, (int64_t)0xffffffff00000000L},
      {(int64_t)0x0000000000000000L, (int64_t)0xffffff0000000000L},
      {(int64_t)0x0000000000000000L, (int64_t)0xffff000000000000L},
      {(int64_t)0x0000000000000000L, (int64_t)0xff00000000000000L},
      {(int64_t)0x0000000000000000L, (int64_t)0x0000000000000000L},
  };
  return masks[i];
}

__m128i _GET_SHUFFLE(_WORD i);
inline __m128i _GET_SHUFFLE(_WORD i) {
  static __m128i masks[17] = {
      {(int64_t)0x0706050403020100L, 0x0f0e0d0c0b0a0908L},
      {(int64_t)0x06050403020100ffL, 0x0e0d0c0b0a090807L},
      {(int64_t)0x050403020100ffffL, 0x0d0c0b0a09080706L},
      {(int64_t)0x0403020100ffffffL, 0x0c0b0a0908070605L},
      {(int64_t)0x03020100ffffffffL, 0x0b0a090807060504L},
      {(int64_t)0x020100ffffffffffL, 0x0a09080706050403L},
      {(int64_t)0x0100ffffffffffffL, 0x0908070605040302L},
      {(int64_t)0x00ffffffffffffffL, 0x0807060504030201L},
      {(int64_t)0xffffffffffffffffL, 0x0706050403020100L},
      {(int64_t)0xffffffffffffffffL, 0x06050403020100ffL},
      {(int64_t)0xffffffffffffffffL, 0x050403020100ffffL},
      {(int64_t)0xffffffffffffffffL, 0x0403020100ffffffL},
      {(int64_t)0xffffffffffffffffL, 0x03020100ffffffffL},
      {(int64_t)0xffffffffffffffffL, 0x020100ffffffffffL},
      {(int64_t)0xffffffffffffffffL, 0x0100ffffffffffffL},
      {(int64_t)0xffffffffffffffffL, 0x00ffffffffffffffL},
      {(int64_t)0xffffffffffffffffL, (int64_t)0xffffffffffffffffL},
  };
  return masks[i];
}

// Treat the word as an number of base 10 digits, one per byte, and calculate
// the number they represent.
unsigned long _CALCULATE_BASE_10(_CHUNK bytes);
inline unsigned long _CALCULATE_BASE_10(_CHUNK bytes) {
  // 16 bytes, 0-9.
  _CHUNK one_ten = _mm_set_epi8(1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10);
  bytes = _mm_maddubs_epi16(bytes, one_ten);
  // 8 16-bit values, 0-99.  Still fit in one byte, so every second byte is zero.
  _CHUNK one_100 = _mm_set_epi8(0, 1,  0, 100, 0, 1, 0, 100, 0, 1, 0, 100, 0, 1, 0, 100);
  bytes = _mm_maddubs_epi16(bytes, one_100);
  // 8 16-bit values, alternating between 0-99 and 0-9900 range.
  bytes += (bytes >> 16);  // Shift is only within 64 bit halves, but that's OK.
  // 4 16-bit values, 0-9999.  Interleaved with junk values.
  bytes &= _REPEAT_32(0x0000ffff);
  // 4 32-bit values, 0-9999.

  alignas(16) uint64_t v[2];
  _mm_store_si128((__m128i*)v, bytes);
  uint64_t hi = v[0];
  uint64_t lo = v[1];

  // See below for explanation of the 2710.
  hi = (hi * 0x271000000001L) >> 32;
  lo = (lo * 0x271000000001L) >> 32;
  return hi * 100000000 + lo;
}

#elif defined(__SSSE3__notnow)

// This mode uses half of an SSE register to process 64 bits at a time.

#define _WORD uint64_t
// We use only half of the __m128i type.
#define _CHUNK __m128i
#define _CHUNK_SIZE 8
#define _REPEAT(value) {(int64_t)(((((uint64_t)0) - 1) / 0xff) * (value)), 0}
#define _REPEAT_16(value) {(int64_t)(((((uint64_t)0) - 1) / 0xffff) * (value)), 0}
#define _REPEAT_32(value) {(int64_t)(((((uint64_t)0) - 1) / 0xffffffff) * (value)), 0}
#define _LOAD_FROM(p) {(int64_t)(*((uint64_t*)(p)) ^ 0x3030303030303030), 0}
#define _ZERO_FIRST_N_BYTES(value, n) ((value) & (ff << ((n) * 8)))
#define _DEFINE_STATIC_CONSTS                                                \
    static const __m128i zeros = _REPEAT(0);                                 \
    static const __m128i nines = _REPEAT(9);                                 \
    static const __m128i ff = _REPEAT(0xff)

// The vector has been xored with 0x30 so that digits are in the range 0-9.
// We do a saturating bytewise subtract so that all digits are 0, and all other
// characters are non-zero, then compare with all zeros.  We then extract all the
// top bits to a 16 bit regular integer.
#define _MAP_OF_NON_DIGITS(value) (_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_subs_epu8((value), nines), zeros)))
#define _MAP_TYPE int
#define _MAP_HAS_NON_DIGITS(value) ((value) != 0xffff)
// We are looking for the index of the least significant 0-bit.  The intrinsic
// finds the least significant 1-bit, so we add 1, which flips the string of 1-bits
// at the end.  The intrinsic returns 1 more than the index, so we have to subtract
// one.
#define _DIGITS_IN_MAP(value) (__builtin_ffs((value) + 1) - 1)

// Shifting an __m128i left just shifts the two 64 bit values independently,
// which is what we want here (the second value is always blank):
#define _SHIFT_LEFT_N_BYTES(value, n) ((value) << ((n * 8) & (_CHUNK_SIZE * 8 - 1)))

// Treat the word as an number of base 10 digits, one per byte, and calculate
// the number they represent.
uint64_t _CALCULATE_BASE_10(_CHUNK bytes);
inline uint64_t _CALCULATE_BASE_10(_CHUNK bytes) {
  // 8 bytes, 0-9.
  _CHUNK one_ten = {0x010a010a010a010a, 0};
  bytes = _mm_maddubs_epi16(bytes, one_ten);
  // 4 16-bit values, 0-99.  Still fit in one byte, so every second byte is zero.
  // 0x64 == 100.
  _CHUNK one_100 = {0x0001006400010064, 0};
  bytes = _mm_maddubs_epi16(bytes, one_100);
  // 4 16-bit values, alternating between 0-99 and 0-9900 range.
  bytes = _mm_hadd_epi16(bytes, bytes);
  // 2 16-bit values, 0-9999.  Various junk in the high bits.

  // Switch to a regular 64 bit value for the last few steps.
  alignas(16) uint64_t v[2];
  _mm_store_si128((__m128i*)v, bytes);
  uint32_t result = v[0];

  result = (result >> 16) + ((result & 0xffff) * 10000);
  return result;
}

#else

// Pure-C fallback mode uses a single 32 bit or 64 bit chunk to process
// 4 or 8 characters at a time (depending on whether you are on a 64 bit
// platform).

#define _WORD size_t
#define _CHUNK unsigned
#define _CHUNK_SIZE (sizeof(unsigned))
// _REPEAT(0xab) will generate a word-sized unsigned constant of the form 0xababababab.
#define _REPEAT(value) (((((unsigned)0) - 1) / 0xff) * (value))
#define _REPEAT_16(value) (((((unsigned)0) - 1) / 0xffff) * (value))
#define _REPEAT_32(value) (((((unsigned)0) - 1) / 0xffffffff) * (value))
#define _LOAD_FROM(p) (*((_CHUNK*)(p)) ^ _REPEAT('0'))

#define _DEFINE_STATIC_CONSTS do { } while(false)

#define _ZERO_FIRST_N_BYTES(value, n) ((value) & (_REPEAT(0xff) << ((n) * 8)))
#define _MAP_OF_NON_DIGITS(value) (((value) + _REPEAT(0x7f - 9)) | (value)) & _REPEAT(0x80)
#define _MAP_TYPE _CHUNK
#define _MAP_HAS_NON_DIGITS(value) ((value) != 0)
#define _DIGITS_IN_MAP(value) ((__builtin_ffsl(value) - 1) >> 3)
#define _SHIFT_LEFT_N_BYTES(value, n) ((value) << ((n * 8) & (_CHUNK_SIZE * 8 - 1)))

// Treat the word as an number of base 10 digits, one per byte, and calculate
// the number they represent.
_WORD _CALCULATE_BASE_10(_CHUNK bytes);
inline _WORD _CALCULATE_BASE_10(_CHUNK bytes) {
  if (_CHUNK_SIZE == 8) {
    /* The low nibbles in the word have a number from 0-9. */
    bytes = bytes * 10 + (bytes >> 8);
    _CHUNK hi = bytes & _REPEAT_32(0x00ff0000);
    bytes = (bytes & _REPEAT_32(0x000000ff)) * 100 + (hi >> 16);
    /* Each 32 bit half of the word has a number from 0-9999.  Since  */
    /* 10000 is 0x2710 we can combine them by multiplying by          */
    /* 0x2710_0000_0001.                                              */
    /* There are some strange formulations here because we can't      */
    /* shift by 32 without getting annoying warnings when compiling   */
    /* for 32 bit platforms, which don't even run this code.          */
    bytes *= 1 + ((_CHUNK)10000 * 0x10000 * 0x10000);
    bytes >>= 16;
    bytes >>= 16;
  } else {
    /* The low nibbles in the word have a number from 0-9. */
    _CHUNK hi =     bytes & _REPEAT_16(0xff00);
    bytes = (bytes & _REPEAT_16(0xff)) * 10 + (hi >> 8);
    /* Each 16 bit half of the word has a number from 0-99. */
    bytes *= 1 + (100 << 16);
    bytes >>= 16;
  }
  return bytes;
}

#endif

#define _MAX_DIGITS(type, is_unsigned) (sizeof(type) == 4 ? 10 : (sizeof(type) == 8 ? (is_unsigned ? 20 : 19) : 39))

_WORD _POWERS_OF_10(_WORD exponent);
inline _WORD _POWERS_OF_10(_WORD exponent) {
  static const _WORD powers[17] = {
        1ULL,  // 0
        10ULL,  // 1
        100ULL,  // 2
        1000ULL,  // 3
        10000ULL,  // 4
        100000ULL,  // 5
        1000000ULL,  // 6
        10000000ULL,  // 7
        100000000ULL,  // 8
        sizeof(size_t) == 4 ? 0 : 1000000000ULL,  // 9
        sizeof(size_t) == 4 ? 0 : 10000000000ULL,  // 10
        sizeof(size_t) == 4 ? 0 : 100000000000ULL,  // 11
        sizeof(size_t) == 4 ? 0 : 1000000000000ULL,  // 12
        sizeof(size_t) == 4 ? 0 : 10000000000000ULL,  // 13
        sizeof(size_t) == 4 ? 0 : 100000000000000ULL,  // 14
        sizeof(size_t) == 4 ? 0 : 1000000000000000ULL,  // 15
        sizeof(size_t) == 4 ? 0 : 10000000000000000ULL,  // 16
  };
  return powers[exponent];
}

#define _UNLIKELY(x) __builtin_expect((x), 1)

#define _DECLARE(name, width, type, declare_args)                            \
const char *name##_##width declare_args;                                     \

#define _DEFINE(name, width, type, utype, is_unsigned, is_null_terminated,   \
                is_exact, has_size, check_overflow, check_input,             \
                declare_args, declare_locals)                                \
const char *name##_##width declare_args;                                     \
inline const char *name##_##width declare_args {                             \
  declare_locals                                                             \
  _DEFINE_STATIC_CONSTS;                                                     \
  if (check_overflow && has_size) {                                          \
    if (end - p < _MAX_DIGITS(type, is_unsigned)) {                          \
      if (check_input) return parse_nooverflow_##width(result, p, end - p);  \
      return parse_nocheck_##width(result, p, end - p);                      \
    }                                                                        \
  }                                                                          \
  if (has_size && _UNLIKELY(end <= p)) return NULL;                          \
  if (is_null_terminated && _UNLIKELY(!*p)) return NULL;                     \
  bool negative = false;                                                     \
  if (!is_unsigned && *p == '-') {                                           \
    negative = true;                                                         \
    p++;                                                                     \
    if (has_size && _UNLIKELY(end <= p)) return NULL;                        \
  }                                                                          \
  const char *digits_start = p;                                              \
  if (check_input && !(_UNLIKELY('0' <= *p && *p <= '9'))) return NULL;      \
  /* Process digits a word at a time.  Assume little endian.              */ \
  /* We will always read aligned words, so we probably need to ignore     */ \
  /* some bytes at the start.  Make them into leading zeros.              */ \
  _WORD ignore = ((size_t)p) & (_CHUNK_SIZE - 1);                            \
  p -= ignore;                                                               \
  _CHUNK bytes = _LOAD_FROM(p);                                              \
  bytes = _ZERO_FIRST_N_BYTES(bytes, ignore);                                \
  utype r;                                                                   \
  if (is_unsigned) {                                                         \
    r = *result;                                                             \
  } else {                                                                   \
    r = 0;                                                                   \
  }                                                                          \
  _WORD digits_added = _CHUNK_SIZE - ignore;                                 \
  while (true) {                                                             \
    p += _CHUNK_SIZE;                                                        \
    /* Check for non-number in input. */                                     \
    _MAP_TYPE end_map = _MAP_OF_NON_DIGITS(bytes);                           \
    if (check_input && _MAP_HAS_NON_DIGITS(end_map)) {                       \
      const char *new_end = p - _CHUNK_SIZE + _DIGITS_IN_MAP(end_map);       \
      if (has_size && is_exact && _UNLIKELY(new_end != end)) return NULL;    \
      else if (is_null_terminated && is_exact && _UNLIKELY(*new_end != 0)) { \
        return NULL;                                                         \
      }                                                                      \
      else if (new_end < end) end = new_end;                                 \
    }                                                                        \
    if (p >= end) {                                                          \
      /* Last word.  We may need to ignore some trailing bytes because we */ \
      /* always read an aligned word at a time.  But if we need to ignore */ \
      /* all the bits we can skip the shifting of `bytes` since we won't  */ \
      /* look at it again.                                                */ \
      _WORD chop_off = p - end;                                              \
      /* This masking of the shift operand is a no-op since the           */ \
      /* instruction does it.                                             */ \
      bytes = _SHIFT_LEFT_N_BYTES(bytes, chop_off);                          \
      digits_added -= chop_off;                                              \
      p -= chop_off;                                                         \
    }                                                                        \
    if (digits_added != 0) {                                                 \
      if (check_overflow) {                                                  \
        r *= _POWERS_OF_10(digits_added - 1);  /* Sneak up to overflow.   */ \
        if (_UNLIKELY(r > (((utype)0) - 1) / 10)) {                          \
          return NULL;  /* Overflow. */                                      \
        }                                                                    \
        r *= 10;                                                             \
      } else {                                                               \
        r *= _POWERS_OF_10(digits_added);                                    \
      }                                                                      \
      _WORD summed = _CALCULATE_BASE_10(bytes);                              \
      if (check_overflow) {                                                  \
        if (sizeof(utype) == 4 && _CHUNK_SIZE == 16) {                       \
          if (_UNLIKELY(summed + r > UINT_MAX)) return NULL;                 \
          r += summed;                                                       \
        } else {                                                             \
          utype new_result = r + summed;                                     \
          /* Catch overflow when we don't have too many digits. */           \
          if (_UNLIKELY(new_result < r)) return NULL;                        \
          r = new_result;                                                    \
        }                                                                    \
      } else {                                                               \
        r += summed;                                                         \
      }                                                                      \
    }                                                                        \
    if (p >= end) {                                                          \
      if (check_overflow) {                                                  \
        if (_UNLIKELY(end - digits_start > _MAX_DIGITS(utype, is_unsigned))) {            \
          /* We may have an overflow.  We need to check how many leading  */ \
          /* zeros there were to be sure.  This path is not very          */ \
          /* optimized - normally we don't have overflows or large        */ \
          /* numbers of leading zeros.                                    */ \
          for (; digits_start < end; digits_start++) {                       \
            if (*digits_start != '0') break;                                 \
          }                                                                  \
          if (_UNLIKELY(end - digits_start >                                 \
                        _MAX_DIGITS(utype, is_unsigned))) {                  \
            return NULL;                                                     \
          }                                                                  \
        }                                                                    \
      }                                                                      \
      if (is_unsigned) {                                                     \
        *result = (type)r;                                                   \
      } else if (!check_overflow) {                                          \
        *result = negative ? -r : r;                                         \
      } else {                                                               \
        utype max = 0;                                                       \
        max = (max - 1) >> 1;                                                \
        if (negative) {                                                      \
          if (_UNLIKELY(r > max + 1)) return NULL;                           \
          *result = -r;                                                      \
        } else {                                                             \
          if (_UNLIKELY(r > max)) return NULL;                               \
          *result = r;                                                       \
        }                                                                    \
      }                                                                      \
      return end;                                                            \
    }                                                                        \
    _CHUNK next_bytes = _LOAD_FROM(p);                                       \
    digits_added = _CHUNK_SIZE;                                              \
    bytes = next_bytes;                                                      \
  }                                                                          \
}

#define _SET(width, type, utype, is_unsigned)        \
  /*                                                          null-terminated exact  has_size check_overflow check_input */ \
  _DECLARE(parse_nooverflow, width, type,                                                                           (type *result, const char *p, size_t size)) \
  _DECLARE(parse_nocheck,    width, type,                                                                           (type *result, const char *p, size_t size)) \
  _DEFINE (parse,            width, type, utype, is_unsigned, false,          false, false,   true,          true,  (type *result, const char *p), const char *end = (const char*)0 - 1;) \
  _DEFINE (parse_all,        width, type, utype, is_unsigned, true,           true,  false,   true,          true,  (type *result, const char *p), const char *end = (const char*)0 - 1;) \
  _DEFINE (parse_n,          width, type, utype, is_unsigned, false,          false, true,    true,          true,  (type *result, const char *p, size_t size), const char* end = p + size; ) \
  _DEFINE (parse_all_n,      width, type, utype, is_unsigned, false,          true,  true,    true,          true,  (type *result, const char *p, size_t size), const char* end = p + size; ) \
  _DEFINE (parse_overflow,   width, type, utype, is_unsigned, false,          false, false,   false,         true,  (type *result, const char *p), const char* end = (const char*)0 - 1; ) \
  _DEFINE (parse_nooverflow, width, type, utype, is_unsigned, false,          true,  true,    false,         true,  (type *result, const char *p, size_t size), const char* end = p + size; ) \
  _DEFINE (parse_novalidate, width, type, utype, is_unsigned, false,          false, true,    true,          false, (type *result, const char *p, size_t size), const char* end = p + size; ) \
  _DEFINE (parse_nocheck,    width, type, utype, is_unsigned, false,          false, true,    false,         false, (type *result, const char *p, size_t size), const char* end = p + size; )

_SET(int, int, unsigned, false)
_SET(long, long, unsigned long, false)
_SET(long_long, long long, unsigned long long, false)
_SET(unsigned, unsigned, unsigned, true)
_SET(unsigned_long, unsigned long, unsigned long, true)
_SET(unsigned_long_long, unsigned long long, unsigned long long, true)
_SET(32, int32_t, uint32_t, false)
_SET(u32, uint32_t, uint32_t, true)
_SET(64, int64_t, uint64_t, false)
_SET(u64, uint64_t, uint64_t, true)
#ifdef __SIZEOF_INT128__
_SET(128, __int128, unsigned __int128, false)
_SET(u128, unsigned __int128, unsigned __int128, true)
#endif

#undef _CHUNK
#undef _CHUNK_SIZE
#undef _DECLARE
#undef _DEFINE
#undef _DEFINE_STATIC_CONSTS
#undef _DIGITS_IN_MAP
#undef _LOAD_FROM
#undef _MAP_HAS_NON_DIGITS
#undef _MAP_OF_NON_DIGITS
#undef _MAP_TYPE
#undef _MAX_DIGITS
#undef _REPEAT_16
#undef _REPEAT_32
#undef _REPEAT
#undef _SET
#undef _SHIFT_LEFT_N_BYTES
#undef _UNLIKELY
#undef _WORD
#undef _ZERO_FIRST_N_BYTES
