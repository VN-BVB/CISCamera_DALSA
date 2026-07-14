@echo off
call "D:\ProgramData\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" amd64
cd /d D:\Code\CISCamera_DALSA
nmake /f Makefile.Release
