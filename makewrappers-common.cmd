@echo off
set CYGWIN_RUNNER=cygwin-runner.exe
set CYGWIN_RUNNER_CONF=cygwin-runner.conf
set CYGWIN_RUNNER_CONF_TARGET=%CYGWIN_PUBLIC%\.cygwin-runner.conf
copy %CYGWIN_RUNNER% %CYGWIN_ROOT%\bin\
if not exist %CYGWIN_PUBLIC%\nul mkdir %CYGWIN_PUBLIC%
if not exist %CYGWIN_RUNNER_CONF_TARGET% copy %CYGWIN_RUNNER_CONF% %CYGWIN_RUNNER_CONF_TARGET%

%MAKEWRAPPER% cygwin.exe
%MAKEWRAPPER% cygpath.exe /usr/bin/cygpath
%MAKEWRAPPER% diff.exe /usr/bin/diff
%MAKEWRAPPER% grep.exe /usr/bin/grep
%MAKEWRAPPER% less.exe /usr/bin/less
%MAKEWRAPPER% tee.exe /usr/bin/tee
%MAKEWRAPPER% tar.exe /usr/bin/tar

%MAKEWRAPPER% ssh.exe /usr/bin/ssh
