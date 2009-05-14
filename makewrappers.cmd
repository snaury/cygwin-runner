@echo off
set CYGWIN_ROOT=C:\cygwin
set MAKEWRAPPER=%CYGWIN_ROOT%\bin\ruby.exe makewrapper
set CYGWIN_RUNNER_CONF=%CYGWIN_ROOT%\bin-public\.cygwin-runner.conf
copy cygwin-runner.exe %CYGWIN_ROOT%\bin\
if not exist %CYGWIN_RUNNER_CONF% copy cygwin-runner.conf %CYGWIN_RUNNER_CONF%

%MAKEWRAPPER% cygwin.exe
%MAKEWRAPPER% cygpath.exe /bin/cygpath
%MAKEWRAPPER% diff.exe /bin/diff
%MAKEWRAPPER% grep.exe /bin/grep
%MAKEWRAPPER% less.exe /bin/less
%MAKEWRAPPER% tee.exe /bin/tee
%MAKEWRAPPER% tar.exe /bin/tar

%MAKEWRAPPER% ssh.exe /bin/ssh
%MAKEWRAPPER% ruby.exe /bin/ruby
%MAKEWRAPPER% perl.exe /bin/perl

%MAKEWRAPPER% cg.exe /bin/cg
%MAKEWRAPPER% git.exe "$DFGitRoot/bin/git"
%MAKEWRAPPER% gitk.exe "$DFGitRoot/bin/gitk"

%MAKEWRAPPER% gem.exe /bin/gem
%MAKEWRAPPER% rake.exe /bin/rake
