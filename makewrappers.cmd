@echo off
set CYGWIN_ROOT=C:\cygwin
set MAKEWRAPPER=%CYGWIN_ROOT%\bin\ruby.exe makewrapper.rb
call makewrappers-common.cmd
