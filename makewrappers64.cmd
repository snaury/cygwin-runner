@echo off
set CYGWIN_ROOT=C:\cygwin64
set CYGWIN_PUBLIC=%CYGWIN_ROOT%\bin-public
set MAKEWRAPPER=python makewrapper.py %CYGWIN_PUBLIC% cygwin-runner-win64.exe
call makewrappers-common.cmd
