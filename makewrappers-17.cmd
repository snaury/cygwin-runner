@echo off
set CYGWIN_ROOT=C:\cygwin-1.7
set MAKEWRAPPER=%CYGWIN_ROOT%\bin\ruby.exe makewrapper.rb --1.7
call makewrappers-common.cmd
