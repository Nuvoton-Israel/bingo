@echo off

set DELIVERABLES=..\deliverables
set WIN_DIR=%DELIVERABLES%\windows
set LINUX_DIR=%DELIVERABLES%\linux
set PROJ_DIR=..\windows_build
REM =========================================
REM Delete executables
REM =========================================
if exist "%WIN_DIR%"      rmdir /S /Q "%WIN_DIR%"
if exist "%LINUX_DIR%"      rmdir /S /Q "%LINUX_DIR%"



REM =========================================
REM Delete outputs
REM =========================================
if exist "%PROJ_DIR%\Debug"                         rmdir /S /Q "%PROJ_DIR%\Debug" 
if exist "%PROJ_DIR%\Release"                       rmdir /S /Q "%PROJ_DIR%\Release"


echo clean ended successfully

