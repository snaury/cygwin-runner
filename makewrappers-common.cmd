@echo off
set CYGWIN_RUNNER=cygwin-runner.exe
set CYGWIN_RUNNER_CONF=cygwin-runner.conf
set CYGWIN_RUNNER_CONF_TARGET=%CYGWIN_ROOT%\bin-public\.cygwin-runner.conf
copy %CYGWIN_RUNNER% %CYGWIN_ROOT%\bin\
if not exist %CYGWIN_RUNNER_CONF_TARGET% copy %CYGWIN_RUNNER_CONF% %CYGWIN_RUNNER_CONF_TARGET%

%MAKEWRAPPER% cygwin.exe
%MAKEWRAPPER% cygpath.exe /bin/cygpath
%MAKEWRAPPER% diff.exe /bin/diff
%MAKEWRAPPER% grep.exe /bin/grep
%MAKEWRAPPER% less.exe /bin/less
%MAKEWRAPPER% tee.exe /bin/tee
%MAKEWRAPPER% tar.exe /bin/tar

%MAKEWRAPPER% ssh.exe /bin/ssh

%MAKEWRAPPER% cg.exe /bin/cg
%MAKEWRAPPER% git.exe "$DFGITROOT/bin/git"
%MAKEWRAPPER% gitk.exe "$DFGITROOT/bin/gitk"
