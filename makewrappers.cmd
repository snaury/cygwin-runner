@echo off
set MAKEWRAPPER=C:\cygwin\bin\ruby.exe makewrapper
copy cygwin-runner.exe C:\cygwin\bin\

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
%MAKEWRAPPER% git.exe "$DFGITROOT/bin/git"
%MAKEWRAPPER% gitk.exe "$DFGITROOT/bin/gitk"
