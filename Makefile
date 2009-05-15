all: cygwin-runner cygwin-runner-win32

cygwin-runner: cygwin-runner.c
	$(CC) -s -o $@ -DCYGWIN_RUNNER_CONF="\"/bin-public/.cygwin-runner.conf\"" $(CFLAGS) $<
	chmod +x $@

cygwin-runner-win32: cygwin-runner-win32.c
	$(CC) -s -o $@ -mno-cygwin -DCYGWIN_RUNNER="\"/bin/cygwin-runner.exe\"" $(CFLAGS) $<
	chmod +x $@
