all: cygwin-runner cygwin-runner-win32 cygwin-runner-win64

CYGWIN_LOCALE=LANG=en_US.UTF-8
CYGWIN_RUNNER=/bin/cygwin-runner.exe
CYGWIN_RUNNER_CONF=/bin-public/.cygwin-runner.conf
CYGWIN_RUNNER_CFLAGS=-DCYGWIN_RUNNER="\"$(CYGWIN_RUNNER)\"" -DCYGWIN_RUNNER_CONF="\"$(CYGWIN_RUNNER_CONF)\"" -DCYGWIN_SUPPORT_REGISTRY
MINGW32_CC=i686-w64-mingw32-gcc.exe -mwin32 -municode -D_UNICODE -DUNICODE
MINGW64_CC=x86_64-w64-mingw32-gcc.exe -mwin32 -municode -D_UNICODE -DUNICODE

cygwin-runner: cygwin-runner.c Makefile
	$(CC) -s -o $@ $(CYGWIN_RUNNER_CFLAGS) $(CFLAGS) $<
	chmod +x $@

cygwin-runner-win32: cygwin-runner-win32.c Makefile
	$(MINGW32_CC) -s -o $@ -DCYGWIN_LOCALE="\"$(CYGWIN_LOCALE)\"" $(CYGWIN_RUNNER_CFLAGS) $(CFLAGS) $<
	chmod +x $@

cygwin-runner-win64: cygwin-runner-win32.c Makefile
	$(MINGW64_CC) -s -o $@ -DCYGWIN_LOCALE="\"$(CYGWIN_LOCALE)\"" $(CYGWIN_RUNNER_CFLAGS) $(CFLAGS) $<
	chmod +x $@
