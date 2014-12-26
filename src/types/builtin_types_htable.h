/* C code produced by gperf version 3.0.4 */
/* Command-line: gperf -t --output-file=lang/src/types/builtin_types_htable.h lang/src/types/builtin_types_htable.gperf  */
/* Computed positions: -k'1-2' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 5 "lang/src/types/builtin_types_htable.gperf"
struct gperf_builtin_type {const char *name; enum builtin_type type; };

#define TOTAL_KEYWORDS 13
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 6
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 26
/* maximum key range = 24, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (str, len)
     register const char *str;
     register unsigned int len;
{
  static const unsigned char asso_values[] =
    {
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 13,
      27, 10, 27, 27,  0, 27,  4, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27,  4, 27, 27,  5, 27, 27, 27, 27,
      15, 27, 27, 27, 27, 10, 10,  0, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27
    };
  return len + asso_values[(unsigned char)str[1]] + asso_values[(unsigned char)str[0]];
}

#ifdef __GNUC__
__inline
#if defined __GNUC_STDC_INLINE__ || defined __GNUC_GNU_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
const struct gperf_builtin_type *
analyzer_string_is_builtin (str, len)
     register const char *str;
     register unsigned int len;
{
  static const struct gperf_builtin_type wordlist[] =
    {
      {""}, {""}, {""},
#line 19 "lang/src/types/builtin_types_htable.gperf"
      {"u64",BUILTIN_UINT_64},
      {""}, {""},
#line 13 "lang/src/types/builtin_types_htable.gperf"
      {"u8",BUILTIN_UINT_8},
#line 22 "lang/src/types/builtin_types_htable.gperf"
      {"f64",BUILTIN_FLOAT_64},
#line 20 "lang/src/types/builtin_types_htable.gperf"
      {"i64",BUILTIN_INT_64},
#line 12 "lang/src/types/builtin_types_htable.gperf"
      {"uint",BUILTIN_UINT},
      {""},
#line 14 "lang/src/types/builtin_types_htable.gperf"
      {"i8",BUILTIN_INT_8},
      {""},
#line 17 "lang/src/types/builtin_types_htable.gperf"
      {"u32",BUILTIN_UINT_32},
      {""}, {""},
#line 15 "lang/src/types/builtin_types_htable.gperf"
      {"u16",BUILTIN_UINT_16},
#line 21 "lang/src/types/builtin_types_htable.gperf"
      {"f32",BUILTIN_FLOAT_32},
#line 18 "lang/src/types/builtin_types_htable.gperf"
      {"i32",BUILTIN_INT_32},
      {""}, {""},
#line 16 "lang/src/types/builtin_types_htable.gperf"
      {"i16",BUILTIN_INT_16},
      {""},
#line 11 "lang/src/types/builtin_types_htable.gperf"
      {"int",BUILTIN_INT},
      {""}, {""},
#line 23 "lang/src/types/builtin_types_htable.gperf"
      {"string",BUILTIN_STRING}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return &wordlist[key];
        }
    }
  return 0;
}
