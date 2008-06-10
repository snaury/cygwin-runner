envupdate_t envupdates[] = {
    { "PATH", "${DFGITROOT}/bin:/usr/local/bin:/usr/bin:/bin:/usr/X11R6/bin:${PATH}" },
    { "MANPATH", "${DFGITROOT}/share/man:/usr/local/man:/usr/share/man:/usr/man" },
    { "INFOPATH", "/usr/local/info:/usr/share/info:/usr/info" },
    { "PS1", "\\[\\e]0;\\w\\a\\]\\n\\[\\e[32m\\]\\u@\\h \\[\\e[33m\\]\\w\\[\\e[0m\\]\\n\\$ " },
    { "CVS_RSH", "/bin/ssh" },
    { "MAKE_MODE", "unix" },
    { NULL, NULL },
};
