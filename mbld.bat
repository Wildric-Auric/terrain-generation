@echo off
if "%~1"=="" goto :help
if "%~1"=="-h" goto :help

:: Define the actions based on the argument passed
if "%~1"=="-mb" (
    call "buildsys/mb.bat"
    goto :eof
)

if "%~1"=="-scp" (
    python "buildsys/scp.py"
    goto :eof
)

if "%~1"=="-r" (
    call "buildsys/r.bat"
    goto :eof
)

if "%~1"=="-b" (
    call "buildsys/b.bat"
    goto :eof
)

if "%~1"=="-br" (
    call "buildsys/br.bat"
    goto :eof
)

if "%~1"=="-mbr" (
    call "buildsys/mb.bat"
    call "buildsys/r.bat"
    goto :eof
)

:: Invalid argument case
echo Invalid option: %~1
goto :help

:help
type "buildsys\\doc.txt"
