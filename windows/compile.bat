@echo off

@rem ============================================================================================================================
set MSBUILD_COMPILER="C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.exe"
if EXIST %MSBUILD_COMPILER% goto :BUILD

set MSBUILD_COMPILER="C:\Program Files\Microsoft Visual Studio 12.0\Common7\IDE\devenv.exe"
if NOT EXIST %MSBUILD_COMPILER% goto error_unable_to_find_compiler_file


:BUILD


@rem Clear read-only attributes from all files
@attrib -R /S


@rem ============================================================================================================================
@rem Build solution

set bingo_PRJ_FILENAME=bingo.sln 

call clean.bat

call %MSBUILD_COMPILER% %bingo_PRJ_FILENAME%  /build Release /out log_1.txt
IF ERRORLEVEL 1 goto :compilation_error

if NOT EXIST ..\deliverables\windows\bingo.exe   goto error_unable_to_find_exe_file

@goto success

:error_unable_to_find_compiler_file
@echo.
@echo ====================== ERROR: CAN'T FIND VC COMPILER  =========================
@echo Error: Change the value of VCCOMPILER_PATH to be the vc compiler file name 
@echo ===============================================================================
EXIT /B 1

:error_unable_to_find_exe_file
@echo.
@echo ======================== ERROR: CAN'T FIND EXE Files ==========================
@echo Error: Cannot find exe file. Probably compilation error
@echo ===============================================================================
EXIT /B 2

:compilation_error
@echo.
@echo ======================== ERROR: CAN'T BUILD PROJECT ===========================
@echo Error: Cannot build bingo.sln
@echo ===============================================================================
EXIT /B 3

:success
@echo.
@echo ===================================================================
@echo **         bingo.sln Build ended successfully              **
@echo ===================================================================
@echo.