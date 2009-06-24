all: cygwin-runner cygwin-runner-win32 cygwin-runner-win32-17

CYGWIN_RUNNER_CFLAGS=-DCYGWIN_RUNNER_CONF="\"/bin-public/.cygwin-runner.conf\"" -DCYGWIN_RUNNER="\"/bin/cygwin-runner.exe\"" -DCYGWIN_SUPPORT_REGISTRY

cygwin-runner: cygwin-runner.c
	$(CC) -s -o $@ $(CYGWIN_RUNNER_CFLAGS) $(CFLAGS) $<
	chmod +x $@

cygwin-runner-win32: cygwin-runner-win32.c
	$(CC) -s -o $@ -mno-cygwin -D_UNICODE -DUNICODE $(CYGWIN_RUNNER_CFLAGS) $(CFLAGS) $<
	chmod +x $@

cygwin-runner-win32-17: cygwin-runner-win32.c
	$(CC) -s -o $@ -mno-cygwin -D_UNICODE -DUNICODE -DCYGWIN_1_7 -DCYGWIN_LOCALE="\"LC_ALL=en_US.UTF-8\"" $(CYGWIN_RUNNER_CFLAGS) $(CFLAGS) $<
	chmod +x $@
