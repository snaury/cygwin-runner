/*
 * cygwin-runner - cygwin programs runner
 *
 * Copyright (C) 2008 Alexey Borzenkov
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include "cygwin-runner-util.h"

/* arguments parsing */

const char* skip_whitespace(const char* p)
{
    char ch;
    while ((ch = *p) && (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'))
        ++p;
    return p;
}

const char* skip_to_quote_end(const char* p, char stop_ch, int can_have_dollar, int can_have_qchar, int can_have_quotes)
{
    char ch;
    while ((ch = *p)) {
        if (!stop_ch) { /* special case to look until whitespace */
            if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
                break;
        } else if (ch == stop_ch)
            break;
        /* no dollar substitution in single quoted strings or ${...} */
        if (can_have_dollar && ch == '$') {
            const char* post_dollar = ++p;
            ch = *p;
            if (ch == '{') { /* variable substitution */
                p = skip_to_quote_end(++p, '}', /* can_have_dollar */ 0, /* can_have_qchar */ 0, /* can_have_quotes */ 0);
                if (*p == '}')
                    ++p;
                else
                    p = post_dollar; /* malformed or stray dollar sign */
                continue;
            }
            if (ch == '(') { /* command substitution */
                p = skip_to_quote_end(++p, ')', /* can_have_dollar */ 1, /* can_have_qchar */ 0, /* can_have_quotes */ 1);
                if (*p == ')')
                    ++p;
                else
                    p = post_dollar; /* malformed or stray dollar sign */
                continue;
            }
            if (ch == '_' || isalpha(ch)) { /* short variable substitution */
                ++p;
                while ((ch = *p) && (ch == '_' || isalnum(ch)))
                    ++p;
                continue;
            }
            continue; /* stray dollar sign */
        }
        if (can_have_qchar && ch == '\\') { /* quoted character */
            if ((ch = *++p))
                ++p;
            continue;
        }
        if (can_have_quotes && (ch == '\'' || ch == '"')) { /* quoted string */
            const char* post_quote = ++p;
            p = skip_to_quote_end(p, ch, /* can_have_dollar */ ch == '"', /* can_have_qchar */ 1, /* can_have_quotes */ 0);
            if (*p == ch)
                ++p;
            else
                p = post_quote; /* malformed */
            continue;
        }
        ++p; /* regular character */
    }
    return p;
}

char* unquote_var(const char* s, const char* e)
{
    char* p;
    char* r;

    p = alloca(e - s + 1);
    memcpy(p, s, e - s);
    p[e - s] = '\0';
    r = getenv(p);
    return strdup(r ? r : "");
}

char* system_execread(const char* cmd);
char* unquote_cmd(const char* s, const char* e)
{
    int i;
    char* p;
    char* r;

    p = alloca(e - s + 1);
    memcpy(p, s, e - s);
    p[e - s] = '\0';

    return system_execread(p);
}

char* unquote_other(const char* s, const char* e, int can_have_dollar, int can_have_qchar, int can_have_quotes)
{
    const char* p;
    char* r;
    char* rr;
    size_t len = 0;
    size_t index = 0;
    string_array* array = string_array_alloc(0);

    for (p = s; p != e;) {
        char ch = *p;
        if (can_have_dollar && ch == '$') {
            const char* post_dollar = ++p;
            ch = *p;
            if (ch == '{') {
                p = skip_to_quote_end(++p, '}', /* can_have_dollar */ 0, /* can_have_qchar */ 0, /* can_have_quotes */ 0);
                if (*p == '}') {
                    r = unquote_var(post_dollar+1, p);
                    array = string_array_push(array, r);
                    len += strlen(r);
                    ++p;
                } else { /* malformed */
                    p = post_dollar;
                    ++len; /* stray dollar */
                }
                continue;
            }
            if (ch == '(') {
                p = skip_to_quote_end(++p, ')', /* can_have_dollar */ 1, /* can_have_qchar */ 0, /* can_have_quotes */ 1);
                if (*p == ')') {
                    r = unquote_cmd(post_dollar+1, p);
                    array = string_array_push(array, r);
                    len += strlen(r);
                    ++p;
                } else { /* malformed */
                    p = post_dollar;
                    ++len; /* stray dollar */
                }
                continue;
            }
            if (ch == '_' || isalpha(ch)) {
                ++p;
                while (p != e && (ch = *p) && (ch == '_' || isalnum(ch)))
                    ++p;
                r = unquote_var(post_dollar, p);
                array = string_array_push(array, r);
                len += strlen(r);
                continue;
            }
            ++len; /* stray dollar */
            continue;
        }
        if (can_have_qchar && ch == '\\') {
            if ((ch = *++p))
                ++p;
            ++len; /* quoted character */
            continue;
        }
        if (can_have_quotes && (ch == '\'' || ch == '"')) {
            const char* post_quote = ++p;
            p = skip_to_quote_end(p, ch, /* can_have_dollar */ ch == '"', /* can_have_qchar */ 1, /* can_have_quotes */ 0);
            if (*p == ch) {
                r = unquote_other(post_quote, p, /* can_have_dollar */ ch == '"', /* can_have_qchar */ 1, /* can_have_quotes */ 0);
                array = string_array_push(array, r);
                len += strlen(r);
                ++p;
            } else {
                p = post_quote; /* malformed */
                ++len;
            }
            continue;
        }
        ++len; /* simple character */
        ++p;
    }

    r = rr = malloc(len + 1);
    for (p = s; p != e;) {
        char ch = *p;
        if (can_have_dollar && ch == '$') {
            const char* post_dollar = ++p;
            ch = *p;
            if (ch == '{') {
                p = skip_to_quote_end(++p, '}', /* can_have_dollar */ 0, /* can_have_qchar */ 0, /* can_have_quotes */ 0);
                if (*p == '}') {
                    len = strlen(array->ptr[index]);
                    memcpy(r, array->ptr[index++], len);
                    r += len;
                    ++p;
                } else { /* malformed */
                    *r++ = '$';
                    p = post_dollar;
                }
                continue;
            }
            if (ch == '(') {
                p = skip_to_quote_end(++p, ')', /* can_have_dollar */ 1, /* can_have_qchar */ 0, /* can_have_quotes */ 1);
                if (*p == ')') {
                    len = strlen(array->ptr[index]);
                    memcpy(r, array->ptr[index++], len);
                    r += len;
                    ++p;
                } else { /* malformed */
                    *r++ = '$';
                    p = post_dollar;
                }
                continue;
            }
            if (ch == '_' || isalpha(ch)) {
                ++p;
                while (p != e && (ch = *p) && (ch == '_' || isalnum(ch)))
                    ++p;
                len = strlen(array->ptr[index]);
                memcpy(r, array->ptr[index++], len);
                r += len;
                continue;
            }
            *r++ = '$';
            continue;
        }
        if (can_have_qchar && ch == '\\') {
            if ((ch = *++p))
                ++p;
            else
                ch = '\\';
            switch(ch) {
                case 't': ch = '\t'; break;
                case 'r': ch = '\r'; break;
                case 'n': ch = '\n'; break;
            }
            *r++ = ch;
            continue;
        }
        if (can_have_quotes && (ch == '\'' || ch == '"')) {
            const char* post_quote = ++p;
            p = skip_to_quote_end(p, ch, /* can_have_dollar */ ch == '"', /* can_have_qchar */ 1, /* can_have_quotes */ 0);
            if (*p == ch) {
                len = strlen(array->ptr[index]);
                memcpy(r, array->ptr[index++], len);
                r += len;
                ++p;
            } else {
                *r++ = ch;
                p = post_quote; /* malformed */
            }
            continue;
        }
        *r++ = ch;
        ++p;
    }
    *r = '\0';
    string_array_free(array);
    return rr;
}

/* subcommand spawner */

char* get_next_argument(const char** ptr)
{
    string_buf* buf;
    const char* s = skip_whitespace(*ptr);
    const char* e = skip_to_quote_end(s, '\0', /* can_have_dollar */ 1, /* can_have_qchar */ 0, /* can_have_quotes */ 1);
    *ptr = skip_whitespace(e);
    if (s == e)
        return NULL;

    return unquote_other(s, e, /* can_have_dollar */ 1, /* can_have_qchar */ 0, /* can_have_quotes */ 1);
}

pid_t system_execredir(const char* cmd, int* in)
{
    pid_t pid;
    int fildes[2];
    string_array* args = string_array_alloc(0);
    char* p;

    while ((p = get_next_argument(&cmd))) {
        //printf("exec arg: '%s'\n", p);
        args = string_array_push(args, p);
    }
    args = string_array_push(args, NULL);

    if (args->len <= 1)
        return -1;
    if (pipe(fildes) == -1) {
        string_array_free(args);
        return -1;
    }

    switch ((pid = vfork())) {
        case -1: /* error */
            close(fildes[0]);
            close(fildes[1]);
            break;
        case 0: /* child */
            if (fildes[1] != 1) {
                (void)dup2(fildes[1], 1);
                (void)close(fildes[1]);
            }
            if (fildes[0] != 1) {
                (void)close(fildes[0]);
            }
            execvp(args->ptr[0], args->ptr);
            exit(127);
            break;
        default:
            *in = fildes[0];
            close(fildes[1]);
            break;
    }
    string_array_free(args);
    return pid;
}

char* system_execread(const char* cmd)
{
    int in;
    pid_t pid;
    size_t len;
    string_buf* rbuf;
    char buf[1024];
    char* s;
    char* e;
    char ch;
    char* r;

    if ((pid = system_execredir(cmd, &in)) <= 0)
        return NULL;
    rbuf = string_buf_alloc(1024);
    while ((len = read(in, buf, 1024)) > 0)
        rbuf = string_buf_append_with_len(rbuf, buf, len);
    waitpid(pid, 0, 0);
    close(in);

    s = (char*)skip_whitespace(rbuf->ptr);
    e = s + strlen(s);
    while (e != s && (ch = e[-1]) && (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'))
        --e;
    r = strndup(s, e - s);
    string_buf_free(rbuf);
    return r;
}

/* shell_subst shortcut */

char* shell_subst(const char* str)
{
    return unquote_other(str, str + strlen(str), /* can_have_dollar */ 1, /* can_have_qchar */ 0, /* can_have_quotes */ 1);
}

/* update environment from file */

void update_env(const char* filename)
{
    FILE* f;
    char* p;
    char* v;
    char line[1024];

    if (!(f = fopen(filename, "r")))
        return;

    while ((p = fgets(line, 1024, f))) {
        while (*p && (*p == ' ' || *p == '\t'))
            ++p;
        if (*p == '#')
            continue; /* comment */
        for (v = p; *v && *v != '\r' && *v != '\n'; ++v)
            /* nothing */;
        *v = '\0';
        for (v = p; *v && *v != '='; ++v)
            /* nothing */;
        if (*v != '=')
            continue; /* line must have VAR=VALUE format */
        v = shell_subst(p);
        p = v + strlen(v);
        if (p != v && p[-1] == '=') {
            p[-1] = '\0';
            unsetenv(v);
            free(v);
        } else {
            //printf("%s\n", v);
            putenv(v); /* DO NOT FREE */
        }
    }

    fclose(f);
}

/* Usage: program [arguments] */

int main(int argc, char** argv)
{
    int i;
    char** args;

    if (argc <= 1) {
        fprintf(stderr, "Usage: %s program [arguments]\n", argv[0]);
        return 1;
    }

#ifdef CYGWIN_RUNNER_CONF
    update_env(CYGWIN_RUNNER_CONF);
#endif

    args = (char**)malloc(sizeof(char*) * (argc));
    args[0] = shell_subst(argv[1]);
    for(i = 2; i < argc; ++i)
        args[i - 1] = argv[i];
    args[argc - 1] = NULL;

    errno = 0;
    int rc = execvp(args[0], args);
    if(rc == -1 && errno)
        fprintf(stderr, "Error: unable to run %s (errno=%d)\n", args[0], errno);
    free(args[0]);
    free(args);
    return rc;
}
