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
#include <vector>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <sys/wait.h>
#include <sys/errno.h>

namespace ulib
{
    namespace detail
    {
        template<typename _String>
        _String strtrim(const _String& what, const _String& whites)
        {
            typename _String::size_type pos1 = what.find_first_not_of(whites);
            if (pos1 == _String::npos)
                return _String();
            typename _String::size_type pos2 = what.find_last_not_of(whites);
            if (pos2 == _String::npos)
                return _String();
            return what.substr(pos1, pos2 - pos1 + 1);
        }
        
        template<typename _Container, typename _String>
        void strtokenize(_Container& rv, const _String& line, const _String& whites, bool enablequotes, int maxtokens, bool enablebackslash)
        {
            _String token;
            for (typename _String::size_type pos = 0; pos != line.size();)
            {
                pos = line.find_first_not_of(whites, pos);
                if (pos == _String::npos)
                    break;
                if (!--maxtokens)
                {
                    rv.push_back(line.substr(pos));
                    break;
                }
                token.resize(0);
                typename _String::value_type ch = line[pos++];
                if (enablequotes && ch == '"')
                {
                    // quoted string
                    while (pos != line.size())
                    {
                        ch = line[pos++];
                        if (ch == '"')
                            break;
                        if (enablebackslash && ch == '\\' && pos != line.size())
                        {
                            ch = line[pos++];
                            if (ch == 't')
                                ch = '\t';
                            else if (ch == 'r')
                                ch = '\r';
                            else if (ch == 'n')
                                ch = '\n';
                            // otherwise the char is a direct quote
                        }
                        token += ch;
                    }
                    rv.push_back(token);
                    continue;
                }
                // normal token
                token += ch;
                typename _String::size_type pos2 = line.find_first_of(whites, pos);
                if (pos2 == _String::npos)
                    pos2 = line.size();
                token += line.substr(pos, pos2 - pos);
                rv.push_back(token);
                pos = pos2;
            }
        }
    }

    inline std::string strtrim(const std::string& what, const std::string& whites = " \t\r\n")
    { return detail::strtrim(what, whites); }

    template<typename _Container>
    inline void strtokenize(_Container& rv, const std::string& line, const std::string& whites = " \t", bool enablequotes = true, int maxtokens = 0, bool enablebackslash = true)
    { detail::strtokenize(rv, line, whites, enablequotes, maxtokens, enablebackslash); }
}

char** system_makeargs(const std::string& cmd)
{
    std::vector<std::string> rv;
    ulib::strtokenize(rv, cmd);
    if (rv.empty()) {
        return NULL;
    }
    char** args = (char**)malloc(sizeof(char*) * (rv.size() + 1));
    for (unsigned i = 0; i <= rv.size(); ++i) {
        if (i != rv.size())
            args[i] = strdup(rv[i].c_str());
        else
            args[i] = NULL;
    }
    return args;
}

void system_freeargs(char** args)
{
    for (unsigned i = 0; args[i]; ++i) {
        free(args[i]);
        args[i] = NULL;
    }
    free(args);
}

pid_t system_execredir(const std::string& cmd, int* in = NULL)
{
    pid_t pid;
    char** args;
    int fildes[2];

    args = system_makeargs(cmd);
    if (!args)
        return -1;
    if (pipe(fildes) == -1) {
        system_freeargs(args);
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
            execvp(args[0], args);
            exit(127);
            break;
        default:
            *in = fildes[0];
            close(fildes[1]);
            break;
    }
    system_freeargs(args);
    return pid;
}

std::string system_execread(const std::string& cmd, bool* error = NULL)
{
    int in;
    pid_t pid;
    if((pid = system_execredir(cmd, &in)) <= 0) {
        /* there was some error */
        if (error)
            *error = true;
        return std::string();
    }

    size_t len;
    char buf[1024];
    std::stringstream r(std::ios::out);
    while((len = read(in, buf, 1024)) > 0) {
        r.rdbuf()->sputn(buf, len);
    }
    waitpid(pid, 0, 0);
    close(in);

    if (error)
        *error = false;
    return r.str();
}

inline void ss_write(std::stringstream& r, const char* p, size_t size)
{
    if (size)
        r.rdbuf()->sputn(p, size);
}

inline void ss_write(std::stringstream& r, const std::string& s)
{
    if (!s.empty())
        r.rdbuf()->sputn(s.c_str(), s.size());
}

inline void ss_write(std::stringstream& r, const char* p)
{
    ss_write(r, p, strlen(p));
}

bool strsubst_next(std::stringstream& r, const char*& s)
{
    if (!s || !*s)
        return false;
    const char* p = s;
    if (*p == '$') { // possible substitution
        ++p;
        const char* rp = p; // restart position
        if (*rp == '{') { // possible variable
            const char* q = p = rp + 1;
            while (*p && *p != '}')
                ++p;
            if (*p == '}') {
                ss_write(r, getenv(std::string(q, p).c_str()));
                s = p + 1;
                return true;
            }
        }
        if (*rp == '(') { // possible command
            const char* q = p = rp + 1;
            while (*p && *p != ')')
                ++p;
            if (*p == ')') {
                ss_write(r, ulib::strtrim(system_execread(std::string(q, p))));
                s = p + 1;
                return true;
            }
        }
        if (isalpha(*p)) { // a simple variable
            const char* q = p = rp;
            while (*p && isalnum(*p))
                ++p;
            ss_write(r, getenv(std::string(q, p).c_str()));
            s = p;
            return true;
        }
        ss_write(r, s, rp-s);
        s = rp;
        return true;
    }
    while (*p) {
        if (*p == '$')
            break;
        if (*p == '\\') {
            size_t l = p[1] ? 2 : 1;
            ss_write(r, p, l);
            p += l;
        } else
            ss_write(r, p++, 1);
    }
    s = p;
    return true;
}

char* strsubst(const char* s)
{
    std::stringstream r(std::ios::out);
    while (strsubst_next(r, s)) {
        // nothing to do here
    }
    return strdup(r.str().c_str());
}

#ifdef TARGET_VARS
#include TARGET_VARS

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

int main(int argc, char** argv)
{
    int i;
    char** args = (char**)malloc(sizeof(char*) * (argc + 1));
    args[0] = strsubst(TARGET);
    for(i = 1; i < argc; ++i)
        args[i] = argv[i];
    args[argc] = NULL;
#ifdef TARGET_VARS
    update_env();
#endif
    errno = 0;
    int rc = execv(args[0], args);
    if(rc == -1 && errno)
        fprintf(stderr, "Error: unable to run %s (errno=%d)\n", args[0], errno);
    free(args);
    return rc;
}
