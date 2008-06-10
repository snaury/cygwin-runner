#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#ifdef TARGET_VARS
typedef struct
{
    const char* envname;
    const char* template;
} envupdate_t;

#include TARGET_VARS

char* parsevar(const char** s, int copy)
{
    char* r;
    size_t l;
    const char* p;
    const char* q;
    if(!s) return NULL;
    p = *s;
    if(*p++ != '$') return NULL; /* must start with $ */
    if(*p++ != '{') return NULL; /* must be in ${var} format */
    q = p;
    while(*p && *p != '}')
        ++p;
    if(*p++ != '}') return NULL; /* malformed */
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

char* substenv(const char* template)
{
    int i;
    const char* p;
    size_t numvalues = 0;
    const char** values;
    size_t* lvalues;
    size_t l = 0;
    char* r;
    char* o;

    /* first pass: find out number of variables */
    for(p = template; *p;) {
        if(*p == '$' && (p == template || p[-1] != '\\')) {
            /* if $ is not escaped it could be variable substitution */
            if(parsevar(&p, 0)) {
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
            char* name = parsevar(&p, 1);
            if(name) {
                values[i] = getenv(name);
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
            if(parsevar(&p, 0)) {
                if(lvalues[i])
                    memcpy(o, values[i], lvalues[i]);
                o += lvalues[i++];
                continue;
            }
        }
        *o++ = *p++;
    }
    *o = 0;

    free(lvalues);
    free(values);
    return r;
}

char* concat(const char* s1, const char* s2, const char* s3)
{
    size_t l1 = strlen(s1);
    size_t l2 = strlen(s2);
    size_t l3 = strlen(s3);
    char* r = malloc(l1 + l2 + l3 + 1);
    r[0] = 0;
    if(s1) strcat(r, s1);
    if(s2) strcat(r, s2);
    if(s3) strcat(r, s3);
    return r;
}

void update_env(void)
{
    envupdate_t* e;
    for(e = envupdates; e->envname; ++e)
    {
        char* v = substenv(e->template);
        char* p = concat(e->envname, "=", v);
        putenv(p);
    }
}
#endif

#ifdef __MINGW32__
typedef const char** args_t;
#else
typedef const char** args_t;
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
    int rc = spawnv(_P_WAIT, args[0], args);
    if(rc == -1 && errno)
        fprintf(stderr, "Error: unable to run %s (errno=%d)\n", args[0], errno);
    free(args);
    return rc;
}
