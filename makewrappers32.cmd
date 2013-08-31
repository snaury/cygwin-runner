@echo off
set CYGWIN_ROOT=C:\cygwin
set CYGWIN_PUBLIC=%CYGWIN_ROOT%\bin-public
set MAKEWRAPPER=python makewrapper.py %CYGWIN_PUBLIC% cygwin-runner-win32.exe
call makewrappers-common.cmd
