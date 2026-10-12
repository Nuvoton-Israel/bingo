echo off

rem ****************************************
rem Delete old results if exist
rem ****************************************
IF EXIST result.txt DEL /F /Q result.txt

rem *************
rem Run the build
rem *************
CALL Compile.bat
IF %ERRORLEVEL% NEQ 0 goto FAILED
IF EXIST ..\deliverables\windows\bingo.pdb DEL /F /Q ..\deliverables\windows\bingo.pdb

mkdir ..\deliverables\linux
XCOPY /s ..\src\* ..\deliverables\linux\src\
xcopy ..\linux_build\* ..\deliverables\linux\linux_build\*

rem *********************************
rem Post result + Gather deliverables
rem *********************************
:PASSED
SET result=Pass
GOTO END

:FAILED
call clean.bat
SET result=Fail

:END
ECHO %result% > result.txt