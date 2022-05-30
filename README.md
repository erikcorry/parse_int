# ParseInt

Various routines for parsing integers.  The integers can be assumed to be
null terminated or terminated with non-integer characters.  Alternatively
you can pass an explicit string size.

Apart from the more flexible API, it's about 40% faster than standard
`strtoull` on a modern Linux (runs in only 60% of the time).

ParseInt uses SIMD methods to process many digits at a time.  There are
SSE2 versions and versions using plain 64 bit values.  Currently the
non-SSE2 versions are fastest, so the SSE2 versions are not activated.
The non-SSE2 version needs a primitive to find the number of trailing
zeros in a machine word.  Clang and GCC have builtins for this and it's
a single instruction on most CPUs.

All loads are word-based with no cleanup loop at the start or end.  It
can optionally check for overflows and other errors.  Some effort has
been put into making the algorithm branchless.  In particular there is
no branch to eliminate the first bytes that are loaded because of alignment,
but which are not part of the input string.  Inevitably there is a branch
when the end of the number is identified.  Currently there are three
branches at the end, which can all be predicted to go the same way as the first
one that detects the end of the string.  Eliminating one of these with a
goto only gives 1% speed improvement, indicating that they predict well with
modern CPUs.

Float parsing also requires strings of digits to be parsed as integers.
When used to replace the naïve parser in the ingestion part of Lemire's
`fast_double_parser` speeds are slightly lower on short string inputs, and
slightly faster on longer inputs, hitting a peak of about 1.4Gbyte/s for float
parsing (vs. 1.2Gbyte/s on the original code for my Ryzen-based hardware).  The
cut-off is around a precision of 10 (9 digits after the decimal point).

# API

This is a header-only C library, also usable from C++.

There are several related methods for each result type.

For unsigned methods the result argument is a read-write argument - if it
already contains a non-zero value then function works as if that value was
textually prepended to the integer being parsed.  If you start with a non-
zero value there is no check for overflow caused by too many digits, only
for a value that is too high despite having an OK number of digits.  You
can get around this by setting the size argument to a value that prevents too
many digits.

For signed types an initial '-' is allowed.

Returns null if no integer was found or for out-of-range.  Otherwise returns a
pointer to the end of the number.

Replace `int` with any of `unsigned`, `long`, `unsigned long`, `long_long`,
`unsigned_long_long`,`int32`, `uint32`, `int64`, `uint64`, `int128`, or
`uint128` (the last two only on 64 bit platforms, currently).

Parses up to the first char that is not integer or to null char:
```C
const inline char *parse_int(int *result, const char *p);
```

Takes a null terminated string and expects whole string to be a number.
Returns null if there is trailing junk after the integer.
```C
const inline char *parse_all_int(int *result, const char *p);
```

Takes an additional size and stops at whatever comes first: a char
that is not an integer or the end of the string.
```C
const inline char *parse_n_int(int *result, const char *p, size_t size);
```

Takes an additional length pointer and returns null if there is trailing junk
after the string.
```C
const inline char *parse_all_n_int(int *result, const char *start, size_t size);
```

Takes an additional length pointer.  Does not check the input for non-
numeric characters or overflow.  The assumption is that this was checked
previously.
```
const inline char *parse_nocheck_int(int *result, const char *start, size_t size);
```

Takes an additional length pointer.  Does not check the input for overflow.
The assumption is that this was checked previously, perhaps by choosing an
integer type that is certain to be big enough.  Both this and
`parse_overflow_int` will ignore overflow, the difference is that this one
takes a size argument.
```C
const inline char *parse_nooverflow_int(int *result, const char *start, size_t size);
```

Does not check the input for overflow.  The assumption is that this will be
checked afterwards by looking at the number of digits.  Both this and
`parse_nooverflow_int` will ignore overflow, the difference is that this one
does not take a size argument.
```C
const inline char *parse_overflow_int(int *result, const char *start);
```

Takes an additional length pointer.  Does not check the input for non-
numeric characters.  The assumption is that this was checked previously.
```
const inline char *parse_novalidate_int(int *result, const char *start, size_t size);
```
