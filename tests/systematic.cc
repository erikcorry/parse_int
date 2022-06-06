#include <inttypes.h>
#include <stdio.h>
#include <type_traits>

#include "../parse_int.h"

int max_digits(int size) {
  if (size == 4) return 10;
  if (size == 16) return 39;
  if (size == 8) return 20;
  return -1;
}

template <typename T>
int dumb_print_number(char* p, T value) {
  const T min_signed = -1 << (sizeof(T) * 8 - 1);
  typedef typename std::make_unsigned<T>::type UNSIGNED;
  char* start = p;
  UNSIGNED value_unsigned;
  if (value < 0) {
    *p++ = '-';
    value_unsigned = -(value + 1);
    value_unsigned++;
  } else {
    value_unsigned = value;
  }
  int i = max_digits(sizeof(T)) - 1;
  while (i > 0) {
    UNSIGNED digit_value = 1;
    for (int j = 0; j < i; j++) digit_value *= 10;
    if (value_unsigned >= digit_value) {
      break;
    }
    i--;
  }
  for (int j = i; j >= 0; j--) {
    UNSIGNED digit_value = 1;
    for (int k = 0; k < j; k++) digit_value *= 10;
    UNSIGNED digit = value_unsigned / digit_value;
    *p++ = '0' + digit;
    value_unsigned -= digit * digit_value;
  }
  *p = 0;
  return p - start;
}

int main() {
  char buffer[256];
  int size = dumb_print_number<int>(buffer, 2147483647);
  printf("int       %s %d\n", buffer, size);
  size = dumb_print_number<int>(buffer, -2147483648);
  printf("int      %s %d\n", buffer, size);
  size = dumb_print_number<int>(buffer, 0);
  printf("int       %s %d\n", buffer, size);
}

template <typename T>
int parse_templated(T *result, char *p) {
  if (sizeof(T) == 4) {
    if (std::is_unsigned<T>::value) {
      return parse_u32(reinterpret_cast<uint32_t*>(result), p);
    } else {
      return parse_32(reinterpret_cast<int32_t*>(result), p);
    }
  } else if (sizeof(T) == 8) {
    if (std::is_unsigned<T>::value) {
      return parse_u64(reinterpret_cast<uint64_t*>(result), p);
    } else {
      return parse_64(reinterpret_cast<int64_t*>(result), p);
    }
  } else if (sizeof(T) == 16) {
    if (std::is_unsigned<T>::value) {
      return parse_u128(reinterpret_cast<uint128_t*>(result), p);
    } else {
      return parse_128(reinterpret_cast<int128_t*>(result), p);
    }
  }
}

template <typename T>
void signed_test() {
  char buffer[256];
  const T min = -1 << (sizeof(T) * 8 - 1);
  const T max = -(min + 1);
  for (int offset = 0; offset < 16; offset++) {
    memset(buffer, '1', sizeof(buffer));
    int len = dumb_print_number(buffer + offset, min);
    buffer[offset + len] = 0;
    T parsed;
    parse_int(&parsed, 
  }
}
