#ifndef STR_H
#define STR_H

#include "ds.h"
#include "iterator.h"
#include <ctype.h>
#include <stdarg.h>

typedef struct {
    size_t len;
    size_t cap;
    char *s;
} String;

#define STRING_NPOS (-1)
#define STRING_ERROR (-2)

__DS_FUNC_PREFIX void string_reserve(String *s, size_t n);
__DS_FUNC_PREFIX void string_append(String *s, const char *other, int len);

#define __str_test_chars_body(f) for(const char *c = s; *c; ++c) { if (!f(*c)) return false; } return true;

#define __str_convert_case_body(f) for(char *c = s; *c; ++c) { *c = f(*c); }

#define __str_find_x_of_body(checkCharsAction, afterCheckCharsAction, mainLoop, prevChar)                    \
    if (!s->len || !chars) return STRING_NPOS;                                                               \
                                                                                                             \
    if (*chars == '\0') return pos;                                                                          \
                                                                                                             \
    if ((pos = modulo(pos, s->len)) < 0) return STRING_ERROR;                                                \
                                                                                                             \
    const char *c;                                                                                           \
                                                                                                             \
    for (c = chars; *c; ++c) {                                                                               \
        if (s->s[pos] == *c) {                                                                               \
            checkCharsAction                                                                                 \
        }                                                                                                    \
    }                                                                                                        \
    afterCheckCharsAction                                                                                    \
    mainLoop {                                                                                               \
        if (s->s[pos] == s->s[prevChar]) continue;                                                           \
        for (c = chars; *c; ++c) {                                                                           \
            if (s->s[pos] == *c) {                                                                           \
                checkCharsAction                                                                             \
            }                                                                                                \
            afterCheckCharsAction                                                                            \
        }                                                                                                    \
    }                                                                                                        \
    return STRING_NPOS;                                                                                      \

#define __str_prefix_table_body(needle, start, len, incr, decr, whileCond) {                                 \
    table = __ds_malloc(sizeof(int) * len);                                                                  \
    const int minIndex = start;                                                                              \
    int cnd = minIndex; /* needle position */                                                                \
    int index = cnd incr 1; /* table position */                                                             \
    table[cnd] = cnd;                                                                                        \
                                                                                                             \
    whileCond {                                                                                              \
        if (needle[index] == needle[cnd]) { /* matching characters */                                        \
            table[index] = cnd incr 1;                                                                       \
            cnd = cnd incr 1;                                                                                \
            index = index incr 1;                                                                            \
        } else { /* not a match */                                                                           \
            if (cnd != minIndex) { /* get previous table index */                                            \
                cnd = table[cnd decr 1];                                                                     \
            } else { /* set it to 0 (default) */                                                             \
                table[index] = minIndex;                                                                     \
                index = index incr 1;                                                                        \
            }                                                                                                \
        }                                                                                                    \
    }                                                                                                        \
}

#define __str_find_body(pos, hsLength, hsOffset, incr, decr, iMin, iMax, jMin, jMax, tableCall, whileFind, result) \
    if (!(needle && s->len)) return STRING_ERROR;                                                            \
                                                                                                             \
    else if (*needle == '\0' || !len_needle) return pos;                                                     \
                                                                                                             \
    else if ((pos = modulo(pos, s->len)) < 0) return STRING_ERROR;                                           \
                                                                                                             \
    if (len_needle < 0) len_needle = strlen(needle);                                                         \
                                                                                                             \
    int len_haystack = hsLength;                                                                             \
    if (len_needle > len_haystack) return STRING_NPOS;                                                       \
                                                                                                             \
    char *haystack = s->s + hsOffset;                                                                        \
    int res = STRING_NPOS;                                                                                   \
    {                                                                                                        \
        int *table;                                                                                          \
        tableCall                                                                                            \
        __str_find_main_loop(haystack, needle, incr, decr, iMin, iMax, jMin, jMax, whileFind,                \
            res = result;                                                                                    \
            break;                                                                                           \
        )                                                                                                    \
        free(table);                                                                                         \
    }                                                                                                        \
    return res;                                                                                              \

#define __str_find_main_loop(haystack, needle, incr, decr, iMin, iMax, jMin, jMax, cond, onSuccess) {        \
    const int iEnd = iMax;                                                                                   \
    int i = iMin, j = jMin;                                                                                  \
    while (cond) {                                                                                           \
        if (haystack[i] == needle[j]) {                                                                      \
            i = i incr 1;                                                                                    \
            j = j incr 1;                                                                                    \
        } else {                                                                                             \
            if (j != jMin) {                                                                                 \
                j = table[j decr 1];                                                                         \
            } else {                                                                                         \
                i = i incr 1;                                                                                \
            }                                                                                                \
        }                                                                                                    \
                                                                                                             \
        if (j == jMax) {                                                                                     \
            onSuccess                                                                                        \
        }                                                                                                    \
    }                                                                                                        \
}


/**
 * The c-string representation of the provided String.
 */
#define string_c_str(str) ((str)->s)


/**
 * The number of characters in the string (analogous to strlen, but O(1) time complexity in this case).
 */
#define string_len(str) ((int) (str)->len)


/**
 * The capacity of the string (maximum size + 1, to account for the null character).
 */
#define string_capacity(str) ((int) (str)->cap)


/**
 * Tests whether the size of the string is 0.
 */
#define string_empty(str) (!((str)->len))


/**
 * Direct access to the char located at index `i` of the string. Does NOT perform bounds checking.
 *
 * @param  i  Index in string.
 */
#define string_index(str, i) (*((str)->s[(i)]))


/**
 * Reference to the string starting at index `i`. Performs bounds checking, and negative indices are allowed.
 *
 * @param  i  Index in string.
 */
__DS_FUNC_PREFIX_INL char *string_at(String *str, int i) {
    int _idx = modulo(i, str->len);
    return (_idx >= 0) ? &(str->s[_idx]) : NULL;
}


/**
 * Char pointer to the front of the string.
 */
#define string_front(str) iter_begin(STR, 0, (str)->s, (str)->len)


/**
 * Char pointer to the back of the string.
 */
#define string_back(str) iter_rbegin(STR, 0, (str)->s, (str)->len)


/**
 * Iterates through each character in the string from beginning to end.
 *
 * @param  chr  Char pointer to use during iteration.
 */
#define string_iter(str, chr) for (chr = string_front(str); chr != iter_end(STR, 0, (str)->s, (str)->len); iter_next(STR, 0, chr))


/**
 * Iterates through each character in the string from end to the beginning.
 *
 * @param  chr  Char pointer to use during iteration.
 */
#define string_riter(str, chr) for (chr = string_back(str); chr != iter_rend(STR, 0, (str)->s, (str)->len); iter_prev(STR, 0, chr))

/* --------------------------------------------------------------------------
 * String iterator macros
 * -------------------------------------------------------------------------- */

#define iter_begin_STR(id, s, n)    ((n) ? &((s)[0]) : NULL)
#define iter_end_STR(id, s, n)      ((n) ? &((s)[n]) : NULL)
#define iter_rbegin_STR(id, s, n)   (n ? &((s)[(n) - 1]) : NULL)
#define iter_rend_STR(id, s, n)     (n ? &((s)[-1]) : NULL)
#define iter_next_STR(id, p)        (++(p))
#define iter_prev_STR(id, p)        (--(p))
#define iter_deref_STR(p)           (*(p))
#define iter_advance_STR(id, p, n)  ((p) += n)
#define iter_dist_STR(id, p1, p2)   ((p2) - (p1))


/**
 * Creates a new, empty string.
 *
 * @return  Pointer to newly created string.
 */
__DS_FUNC_PREFIX String *string_new(void) {
    String *s = __ds_calloc(1, sizeof(String));
    string_reserve(s, 64);
    s->s[0] = 0;
    return s;
}


/**
 * Creates a new string from a c-string `str`.
 * 
 * @param   str  C-string.
 *
 * @return       Pointer to newly created string.
 */
__DS_FUNC_PREFIX String *string_new_fromCStr(const char *str) {
    String *s = string_new();
    string_append(s, str, strlen(str));
    return s;
}


/**
 * Creates a new string as a copy of `other`.
 * 
 * @param   other  Pointer to existing `String`.
 *
 * @return         Pointer to newly created string.
 */
__DS_FUNC_PREFIX String *string_createCopy(const String *other) {
    String *s = string_new();
    string_append(s, other->s, other->len);
    return s;
}


/**
 * Frees memory allocated to the string struct.
 */
__DS_FUNC_PREFIX_INL void string_free(String *s) {
    if (s->cap) {
        free(s->s);
    }
    free(s);
}


/**
 * Request a change in capacity (maximum size) to `n`.
 *
 * @param  n  New capacity.
 */
__DS_FUNC_PREFIX void string_reserve(String *s, size_t n) {
    if (n <= s->cap) return;

    size_t val = s->cap ? s->cap : 1;
    while (val < n) { /* double the capacity from what it was before */
        val <<= 1;
    }
    char *tmp = __ds_realloc(s->s, val);
    s->s = tmp;
    s->cap = val;
}


/**
 * Resizes the string to be `n` characters long. If this is less than the current size, all but the 
 * first `n` characters are removed. If this is greater than or equal to the current size, the provided 
 * character `c` is appended.
 *
 * @param  n  The new size.
 * @param  c  Character to append.
 */
__DS_FUNC_PREFIX_INL void string_resize(String *s, size_t n, char c) {
    if (n > s->len) {
        string_reserve(s, n + 1);
        memset(&s->s[s->len], c, n - s->len);
    }
    s->s[n] = 0;
    s->len = n;
}


/**
 * Removes all characters, leaving the string with a size of 0.
 */
__DS_FUNC_PREFIX_INL void string_clear(String *s) {
    if (!(s->s)) return;
    s->s[0] = 0;
    s->len = 0;
}


/**
 * Removes `n` characters from the string, starting at index `start`.
 *
 * @param  start  The first index to delete.
 * @param  n      The number of characters to delete. If this is -1, all characters from `start`
 *                  until the end will be removed.
 */
__DS_FUNC_PREFIX void string_erase(String *s, int start, int n) {
    if (!n || s->len == 0) return;

    if ((start = modulo(start, s->len)) < 0) return;

    if (n < 0) {
        n = (int) s->len - start;
    } else {
        n = min(n, (int) s->len - start);
    }

    int end = start + n;
    
    if (end < string_len(s)) { /* move any characters after end to start */
        memmove(&s->s[start], &s->s[end], s->len - (size_t) end);
    }
    s->len -= (size_t) n;
    s->s[s->len] = 0;
}


/**
 * If the string's capacity is greater than its length plus the null terminator, reallocates only 
 * enough space to fit all characters.
 */
__DS_FUNC_PREFIX_INL void string_shrink_to_fit(String *s) {
    if (s->len == 0 || s->len + 1 == s->cap || s->cap <= 64) return;
    char *tmp = __ds_realloc(s->s, s->len + 1); /* realloc only enough space for string and '\0' */
    s->cap = s->len + 1;
    s->s = tmp;
}


/**
 * Appends `c` to the end of the string.
 *
 * @param  c  Character to append.
 */
__DS_FUNC_PREFIX_INL void string_push_back(String *s, char c) {
    if (c == 0) return;
    string_reserve(s, s->len + 1);
    s->s[s->len++] = c;
    s->s[s->len] = 0;
}


/**
 * Removes the last character, if the string is not empty.
 */
__DS_FUNC_PREFIX_INL void string_pop_back(String *s) {
    if (!s->len) return;
    s->s[s->len-- - 1] = 0;
}


/**
 * Replaces characters in the string, starting at `pos`, with `len` characters from `other`.
 *
 * @param  pos    Index in the string where the replacement will occur.
 * @param  other  C-string used as the replacement.
 * @param  len    Number of characters from `other` that will be used. If this is -1, all characters
 *                  from `other` will be used.
 */
__DS_FUNC_PREFIX void string_replace(String *s, int pos, const char *other, int len) {
    if (!other || (*other == '\0') || !len) return;

    if (pos >= (int) s->len) {
        string_append(s, other, len);
        return;
    }

    if ((pos = modulo(pos, s->len)) < 0) return;

    if (len < 0) len = strlen(other);

    string_reserve(s, (size_t) (pos + len + 1));
    memcpy(&s->s[pos], other, len);

    if (pos + len > (int) s->len) {
        s->len = (size_t) (pos + len);
        s->s[s->len] = 0;
    }
}


/**
 * Inserts `len` characters from `other` into this string before `pos`.
 *
 * @param  pos    Index in this string before which characters will be inserted. If this is `STRING_END`,
 *                  characters from `other` will be appended to this string.
 * @param  other  C-string from which characters will be inserted.
 * @param  len    Number of characters from `other` to insert. If this is -1, all characters from
 *                  `other` will be used.
 */
__DS_FUNC_PREFIX void string_insert(String *s, int pos, const char *other, int len) {
    if (!other || (*other == '\0') || !len) return;

    if (pos >= (int) s->len) {
        string_append(s, other, len);
        return;
    }

    if ((pos = modulo(pos, s->len)) < 0) return;

    if (len < 0) len = strlen(other);

    string_reserve(s, s->len + (size_t) len + 1);
    memmove(&s->s[pos + len], &s->s[pos], s->len - (size_t) pos);
    memcpy(&s->s[pos], other, len);
    s->len += (size_t) len;
    s->s[s->len] = 0;
}


/**
 * Appends `len` characters from `other` to the end of this string.
 *
 * @param  other  C-string from which characters will be inserted.
 * @param  len    Number of characters from `other` to insert. If this is -1, all characters from
 *                  `other` will be used.
 */
__DS_FUNC_PREFIX void string_append(String *s, const char *other, int len) {
    if (!other || (*other == '\0') || !len) return;

    if (len < 0) len = strlen(other);

    string_reserve(s, s->len + (size_t) len + 1);
    memcpy(&s->s[s->len], other, len);
    s->len += (size_t) len;
    s->s[s->len] = 0;
}


/**
 * Inserts a format string `format` into this string before `pos`.
 *
 * @param  pos     Index in this string before which characters will be inserted. If this is `STRING_END`,
 *                   characters from `format` will be appended to this string.
 * @param  format  Format string.
 */
__DS_FUNC_PREFIX_INL void string_printf(String *s, int pos, const char *format, ...) {
    va_list args;
    int n = 0;
    size_t buf_size = 256;
    char *buf = __ds_malloc(buf_size);

    while (1) {
        va_start(args, format);
        n = vsnprintf(buf, buf_size, format, args);
        va_end(args);

        if ((n > -1) && ((size_t) n < buf_size)) { /* vsnprintf was successful */
            string_insert(s, pos, buf, n);
            free(buf);
            return;
        } else if (n > -1) { /* buffer was too small */
            buf_size <<= 1;
            char *temp = __ds_realloc(buf, buf_size);
            buf = temp;
        } else { /* some error with vsnprintf, stop the function */
            free(buf);
            return;
        }
    }
}


/**
 * Finds the first occurrence of the first `len_needle` characters from `needle` in this string 
 * starting at `start_pos`.
 *
 * @param   start_pos   First index in the string to consider for the search.
 * @param   needle      Substring to find.
 * @param   len_needle  Number of characters to match from needle. If this is -1, all characters from
 *                        `needle` will be used.
 *
 * @return              The index in this string, corresponding to needle[0], where `needle` was found,
 *                      `STRING_NPOS` if it was not found, or `STRING_ERROR` if an error occurred.
 */
__DS_FUNC_PREFIX int string_find(String *s, int start_pos, const char *needle, int len_needle) {
    __str_find_body(start_pos, (int) s->len - start_pos, start_pos, +, -, 0, len_haystack, 0, len_needle, __str_prefix_table_body(needle, 0, len_needle, +, -, while(index < len_needle)), i < iEnd, start_pos + (i - j))
}


/**
 * Finds the last occurrence of the first `len_needle` characters from `needle` in this string 
 * ending at `end_pos`.
 *
 * @param   end_pos     Last index in the string to consider for the search.
 * @param   needle      Substring to find.
 * @param   len_needle  Number of characters to match from needle. If this is -1, all characters from
 *                        `needle` will be used.
 *
 * @return              The index in this string, corresponding to needle[0], where `needle` was found,
 *                        `STRING_NPOS` if it was not found, or `STRING_ERROR` if an error occurred.
 */
__DS_FUNC_PREFIX int string_rfind(String *s, int end_pos, const char *needle, int len_needle) {
    __str_find_body(end_pos, end_pos + 1, 0, -, +, end_pos, -1, len_needle - 1, -1, __str_prefix_table_body(needle, (len_needle-1), len_needle, -, +, while(index > -1)), i > iEnd, i + 1)
}


/**
 * Finds the first index at or after `pos` where one of the characters in `chars` was found.
 *
 * @param   pos    First index in the string to consider.
 * @param   chars  C-string of characters to look for.
 *
 * @return         The first index at or after `pos` where one of the supplied characters was
 *                   found, `STRING_NPOS` if it was not found, or `STRING_ERROR` if an error occurred.
 */
int string_find_first_of(String *s, int pos, const char *chars) {
    __str_find_x_of_body(return pos;, , for(++pos; pos < (int) s->len; ++pos), pos-1)
}


/**
 * Finds the last index at or before `pos` where one of the characters in `chars` was found.
 *
 * @param   pos    Last index in the string to consider.
 * @param   chars  C-string of characters to look for.
 *
 * @return         The last index at or before `pos` where one of the supplied characters was
 *                   found, `STRING_NPOS` if it was not found, or `STRING_ERROR` if an error occurred.
 */
int string_find_last_of(String *s, int pos, const char *chars) {
    __str_find_x_of_body(return pos;, , for(--pos; pos >= 0; --pos), pos+1)
}


/**
 * Finds the first index at or after `pos` where one of the characters in `chars` was not found.
 *
 * @param   pos    First index in the string to consider.
 * @param   chars  C-string of characters to look for.
 *
 * @return         The first index at or after `pos` where a different character was found,
 *                   `STRING_NPOS` if it was not found, or `STRING_ERROR` if an error occurred.
 */
int string_find_first_not_of(String *s, int pos, const char *chars) {
    __str_find_x_of_body(break;, if (!(*c)) return pos;, for(++pos; pos < (int) s->len; ++pos), pos-1)
}


/**
 * Finds the last index at or before `pos` where one of the characters in `chars` was not found.
 *
 * @param   pos    Last index in the string to consider.
 * @param   chars  C-string of characters to look for.
 *
 * @return         The last index at or before `pos` where a different character was found,
 *                   `STRING_NPOS` if it was not found, or `STRING_ERROR` if an error occurred.
 */
int string_find_last_not_of(String *s, int pos, const char *chars) {
    __str_find_x_of_body(break;, if (!(*c)) return pos;, for(--pos; pos >= 0; --pos), pos+1)
}


/**
 * Creates a substring from this string with `n` characters, starting at `start` and moving to
 * the next character to include with a step size of `step_size`.
 *
 * @param   start      Index where the substring should start.
 * @param   n          Maximum number of characters in the substring. -1 implies to include as many
 *                       elements as `start` and `step_size` allow.
 * @param   step_size  How to adjust the index when copying characters. 1 means move forward 1 index
 *                       at a time, -1 means move backwards one index at a time, 2 would mean every
 *                       other index, etc.
 *
 * @return             Newly allocated `String`, or NULL if an error occurred.
 */
__DS_FUNC_PREFIX String *string_substr(String *s, int start, int n, int step_size) {
    if (!s->len || !n) return NULL;

    if (step_size == 0) {
        step_size = 1;
    }

    if ((start = modulo(start, s->len)) < 0) return NULL;

    String *sub = string_new();
    int end;

    if (step_size < 0) {
        end = (n < 0) ? -1 : max(-1, start + (n * step_size));
        for (; start > end; start += step_size) {
            string_push_back(sub, s->s[start]);
        }
    } else {
        end = (n < 0) ? string_len(s) : min(string_len(s), start + (n * step_size));
        for (; start < end; start += step_size) {
            string_push_back(sub, s->s[start]);
        }
    }
    return sub;
}


/**
 * Splits this string into substrings based on `delim` and stores them as newly allocated `String`s 
 * in an array. A sentinel value of NULL is placed at the last index of the array.
 *
 * @param   delim  The delimiter to use to split the string.
 *
 * @return         The array of pointers to `String`, each of which is a substring of this string, or
 *                 NULL if an error occurred.
 */
__DS_FUNC_PREFIX String **string_split(String *s, const char *delim) {
    if (!delim || *delim == '\0' || !s->len) return NULL;

    const int len_delim = strlen(delim);
    int arr_len = 0, *positions = __ds_calloc(8, sizeof(int));

    {
        int pos_size = 8, index = 0;
        int *table;
        __str_prefix_table_body(delim, 0, len_delim, +, -, while(index < len_delim))
    
        __str_find_main_loop(s->s, delim, +, -, 0, string_len(s), 0, len_delim, i < iEnd,                    \
            if (index == pos_size) {                                                                         \
                pos_size <<= 1;                                                                              \
                int *temp = __ds_realloc(positions, pos_size * sizeof(int));                                 \
                positions = temp;                                                                            \
                memset(&positions[index + 1], 0, (pos_size - index + 1) * sizeof(int));                      \
            }                                                                                                \
            positions[index++] = (i - j);                                                                    \
            j = table[j - 1];                                                                                \
        )
        if (index == pos_size) {
            int *temp = __ds_realloc(positions, (pos_size + 1) * sizeof(int));
            positions = temp;
        }
        positions[index++] = -1;
        arr_len = index;
        free(table);
    }
    
    String **arr = __ds_malloc((arr_len + 1) * sizeof(String *));
    String *substring = NULL;
    int i = 0, start = 0, end = positions[0];

    while (end != -1) {
        substring = string_new();
        string_insert(substring, 0, &s->s[start], end - start);
        arr[i++] = substring;
        start = end + len_delim;
        end = positions[i];
    }

    substring = string_new();
    string_insert(substring, 0, &s->s[start], string_len(s) - start);
    arr[i++] = substring;
    arr[arr_len] = NULL;
    free(positions);
    return arr;    
}


/**
 * Frees the memory allocated by `string_split`.
 *
 * @param  arr  Array allocated by `string_split`.
 */
__DS_FUNC_PREFIX_INL void string_split_free(String **arr) {
    for (String **s = arr; *s; ++s) {
        string_free(*s);
    }
    free(arr);
}


/**
 * @param   s  C-string.
 *
 * @return     Whether or not all characters in `s` are alphanumeric.
 */
__DS_FUNC_PREFIX_INL bool isAlphaNum(const char *s) {
    __str_test_chars_body(isalnum)
}


/**
 * @param   s  C-string.
 *
 * @return     Whether or not all characters in `s` are letters.
 */
__DS_FUNC_PREFIX_INL bool isAlpha(const char *s) {
    __str_test_chars_body(isalpha)
}


/**
 * @param   s  C-string.
 *
 * @return     Whether or not all characters in `s` are digits.
 */
__DS_FUNC_PREFIX_INL bool isDigit(const char *s) {
    __str_test_chars_body(isdigit)
}


/**
 * Converts all characters in `s` to lowercase.
 *
 * @param  s  C-string.
 */
__DS_FUNC_PREFIX_INL void toLowercase(char *s) {
    __str_convert_case_body(tolower)
}


/**
 * Converts all characters in `s` to uppercase.
 *
 * @param  s  C-string.
 */
__DS_FUNC_PREFIX_INL void toUppercase(char *s) {
    __str_convert_case_body(toupper)
}

#endif /* STR_H */
