all: cygwin-runner cygwin-runner-win32 cygwin-runner-win32-17

CYGWIN_LOCALE=LANG=en_US.UTF-8
CYGWIN_RUNNER=/bin/cygwin-runner.exe
CYGWIN_RUNNER_CONF=/bin-public/.cygwin-runner.conf
CYGWIN_RUNNER_CFLAGS=-DCYGWIN_RUNNER="\"$(CYGWIN_RUNNER)\"" -DCYGWIN_RUNNER_CONF="\"$(CYGWIN_RUNNER_CONF)\"" -DCYGWIN_SUPPORT_REGISTRY

cygwin-runner: cygwin-runner.c Makefile
	$(CC) -s -o $@ $(CYGWIN_RUNNER_CFLAGS) $(CFLAGS) $<
	chmod +x $@

cygwin-runner-win32: cygwin-runner-win32.c Makefile
	$(CC) -s -o $@ -mno-cygwin -D_UNICODE -DUNICODE $(CYGWIN_RUNNER_CFLAGS) $(CFLAGS) $<
	chmod +x $@

cygwin-runner-win32-17: cygwin-runner-win32.c Makefile
	$(CC) -s -o $@ -mno-cygwin -D_UNICODE -DUNICODE -DCYGWIN_1_7 -DCYGWIN_LOCALE="\"$(CYGWIN_LOCALE)\"" $(CYGWIN_RUNNER_CFLAGS) $(CFLAGS) $<
	chmod +x $@
