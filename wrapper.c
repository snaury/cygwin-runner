/*
 * wrapper - cygwin programs wrapper
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#ifdef TARGET_VARS
#include TARGET_VARS

int system_execredir(const char* cmd, int* in)
{
    int pid;
    int fildes[2];

    if(pipe(fildes) == -1)
        return -1;
    switch((pid = fork())) {
        case -1: /* error */
            close(fildes[0]);
            close(fildes[1]);
            break;
        case 0: /* child */
            close(fildes[0]);
            dup2(fildes[1], 1);
            close(fildes[1]);
            exit(execlp("/bin/sh", "sh", "-c", cmd));
            break;
        default:
            close(fildes[1]);
            *in = fildes[0];
            break;
    }
    return pid;
}

typedef struct
{
    size_t len;
    char* buf;
    void* next;
} readchain_t;

#define REDIR_BUF_SIZE 1024
char* system_execread(const char* cmd)
{
    int in;
    int pid;
    size_t len;
    char buf[REDIR_BUF_SIZE];
    readchain_t* first = NULL;
    readchain_t* last = NULL;
    readchain_t* cur;
    char* r;
    char* o;

    if((pid = system_execredir(cmd, &in)) <= 0) {
        /* there was some error */
        return NULL;
    }

    while((len = read(in, buf, REDIR_BUF_SIZE)) > 0) {
        cur = malloc(sizeof(readchain_t));
        cur->len = len;
        cur->buf = malloc(len);
        cur->next = NULL;
        memcpy(cur->buf, buf, len);
        if(!first) {
            first = cur;
            last = cur;
        } else {
            last->next = cur;
            last = cur;
        }
    }
    waitpid(pid);
    close(in);

    len = 0;
    for(cur = first; cur; cur = cur->next) {
        len += cur->len;
    }

    o = r = malloc(len + 1);
    for(cur = first; cur; cur = cur->next) {
        memcpy(o, cur->buf, cur->len);
        o += cur->len;
    }
    *o = 0;

    return r;
}

char* strstrip(const char* s, int dofree)
{
    const char* p;
    const char* e;
    size_t len;
    char* r;

    if(!s)
        return NULL;
    p = s;
    e = s + strlen(s);
    while(p != e && isspace(*p))
        ++p;
    while(e != p && isspace(e[-1]))
        --e;
    if(p == e)
        return NULL;
    len = e - p;
    r = malloc(len + 1);
    memcpy(r, p, len);
    r[len] = 0;
    if(dofree)
        free((void*)s);
    return r;
}

char* strcopy(const char* s)
{
    size_t len;
    char* r;

    if(!s)
        return NULL;
    len = strlen(s);
    r = malloc(len + 1);
    memcpy(r, s, len);
    r[len] = 0;
    return r;
}

char* do_parsevar(const char** s, int copy, char lp, char rp)
{
    char* r;
    size_t l;
    const char* p;
    const char* q;
    if(!s) return NULL;
    p = *s;
    if(*p++ != '$') return NULL; /* must start with $ */
    if(*p++ != lp) return NULL; /* must be in ${var} format */
    q = p;
    while(*p && *p != rp)
        ++p;
    if(*p++ != rp) return NULL; /* malformed */
    *s = p;
    if (copy) {
        size_t l = p - q - 1;
        char* r = malloc(l + 1);
        memcpy(r, q, l);
        r[l] = 0;
        return r;
    }
    return (char*)q;
}

char* parsevar_env(const char** s, int copy)
{
    return do_parsevar(s, copy, '{', '}');
}

char* parsevar_cmd(const char** s, int copy)
{
    return do_parsevar(s, copy, '(', ')');
}

char* strsubst(const char* template)
{
    int i;
    const char* p;
    size_t numvalues = 0;
    char* name;
    char** values;
    size_t* lvalues;
    size_t l = 0;
    char* r;
    char* o;

    /* first pass: find out number of variables */
    for(p = template; *p;) {
        if(*p == '$' && (p == template || p[-1] != '\\')) {
            /* if $ is not escaped it could be variable substitution */
            if(parsevar_env(&p, 0) || parsevar_cmd(&p, 0)) {
                ++numvalues;
                continue;
            }
        }
        ++p;
    }

    /* second pass: fetch variable values */
    values = malloc(sizeof(const char*) * numvalues);
    lvalues = malloc(sizeof(int*) * numvalues);
    for(i = 0, p = template; *p;) {
        if(*p == '$' && (p == template || p[-1] != '\\')) {
            /* if $ is not escaped it could be variable substitution */
            if((name = parsevar_env(&p, 1))) {
                values[i] = strcopy(getenv(name));
                lvalues[i] = values[i] ? strlen(values[i]) : 0;
                l += lvalues[i++];
                free(name);
                continue;
            }
            if((name = parsevar_cmd(&p, 1))) {
                values[i] = strstrip(system_execread(name), 1);
                lvalues[i] = values[i] ? strlen(values[i]) : 0;
                l += lvalues[i++];
                free(name);
                continue;
            }
        }
        ++p;
        ++l;
    }

    /* third pass: construct the result */
    o = r = malloc(l + 1);
    for(i = 0, p = template; *p;) {
        if(*p == '$' && (p == template || p[-1] != '\\')) {
            /* if $ is not escaped it could be variable substitution */
            if(parsevar_env(&p, 0) || parsevar_cmd(&p, 0)) {
                if(lvalues[i])
                    memcpy(o, values[i], lvalues[i]);
                o += lvalues[i++];
                continue;
            }
        }
        *o++ = *p++;
    }
    *o = 0;

    /* cleanup */
    for(i = 0; i < numvalues; ++i) {
        if(values[i])
            free(values[i]);
    }
    free(lvalues);
    free(values);
    return r;
}

void update_env(void)
{
    const char** e;
    for(e = envupdates; *e; ++e)
    {
        char* v = strsubst(*e);
        putenv(v);
    }
}
#endif

#ifdef __MINGW32__
typedef const char** args_t;
#else
typedef char** args_t;
#endif

int main(int argc, char** argv)
{
    int i;
    args_t args = malloc(sizeof(char*) * (argc + 1));
    args[0] = TARGET;
    for(i = 1; i < argc; ++i)
        args[i] = argv[i];
    args[argc] = NULL;
#ifdef TARGET_VARS
    update_env();
#endif
    errno = 0;
#ifdef __MINGW32__
    int rc = spawnv(_P_WAIT, args[0], args);
#else
    int rc = execv(args[0], args);
#endif
    if(rc == -1 && errno)
        fprintf(stderr, "Error: unable to run %s (errno=%d)\n", args[0], errno);
    free(args);
    return rc;
}
