#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OFFSET_OF(type, member) ((size_t)(&((type*)0)->member))

/* string buffer */
typedef struct string_buf
{
    size_t maxlen;
    size_t len;
    char ptr[1];
} string_buf;

string_buf* string_buf_alloc(size_t maxlen)
{
    string_buf* buf = malloc(OFFSET_OF(string_buf, ptr) + maxlen + 1);
    if (!buf)
        return buf;
    buf->maxlen = maxlen;
    buf->ptr[buf->len = 0] = '\0';
    return buf;
}

void string_buf_free(string_buf* buf)
{
    if (buf)
        free(buf);
}

string_buf* string_buf_new_with_len(const char* str, size_t len)
{
    string_buf* buf = malloc(OFFSET_OF(string_buf, ptr) + len + 1);
    if (!buf)
        return buf;
    buf->maxlen = len;
    memcpy(buf->ptr, str, len);
    buf->ptr[buf->len = len] = '\0';
    return buf;
}

string_buf* string_buf_new(const char* str)
{
    size_t len = str ? strlen(str) : 0;
    return string_buf_new_with_len(str, len);
}

string_buf* string_buf_append_with_len(string_buf* buf, const char* str, size_t len)
{
    if (!str || !len)
        return buf;
    if (!buf)
        return string_buf_new_with_len(str, len);
    if (buf->len + len > buf->maxlen) {
        size_t newmaxlen = (buf->len + len) > (buf->maxlen * 2) ? (buf->len + len) : (buf->maxlen * 2);
        buf = realloc(buf, OFFSET_OF(string_buf, ptr) + newmaxlen + 1);
        if (!buf)
            return buf;
        buf->maxlen = newmaxlen;
    }
    memcpy(buf->ptr + buf->len, str, len);
    buf->ptr[buf->len += len] = '\0';
    return buf;
}

string_buf* string_buf_append(string_buf* buf, const char* str)
{
    size_t len = str ? strlen(str) : 0;
    return len ? string_buf_append_with_len(buf, str, len) : buf;
}

/* string array */
typedef struct string_array
{
    size_t maxlen;
    size_t len;
    char* ptr[1];
} string_array;

string_array* string_array_alloc(size_t maxlen)
{
    string_array* array = malloc(OFFSET_OF(string_array, ptr) + maxlen * sizeof(char*));
    array->maxlen = 0;
    array->len = 0;
    return array;
}

string_array* string_array_push(string_array* array, char* str)
{
    if (!array) {
        array = string_array_alloc(1);
        if (!array)
            return array;
    }
    if (array->len + 1 > array->maxlen) {
        size_t newmaxlen = (array->len + 1) > (array->maxlen * 2) ? (array->len + 1) : (array->maxlen * 2);
        array = realloc(array, OFFSET_OF(string_array, ptr) + newmaxlen * sizeof(char*));
        if (!array)
            return array;
        array->maxlen = newmaxlen;
    }
    array->ptr[array->len++] = str;
    return array;
}

void string_array_free(string_array* array)
{
    size_t i;
    char** p;
    if (!array)
        return;
    for (i = 0, p = array->ptr; i < array->len; ++i, ++p) {
        if (*p)
            free(*p);
    }
    free(array);
}
