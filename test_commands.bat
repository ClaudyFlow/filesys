@echo off
setlocal enabledelayedexpansion

echo ================================================
echo   NACHOS Filesystem Command Test Suite
echo ================================================
echo.

set "MAIN_EXE=D:\filesys\build\main.exe"
set "FS_FILE=D:\filesys\build\filesystem"
set "TEMP_CMDS=%TEMP%\nachos_test_cmds.txt"
set "TEMP_OUT=%TEMP%\nachos_test_out.txt"

if not exist "%FS_FILE%" (
    echo [SETUP] Creating filesystem...
    D:\filesys\build\mkfs.exe > nul
    copy /Y "D:\filesys\filesystem" "%FS_FILE%" > nul 2>&1
)

echo [SETUP] Test started at %date% %time%
echo.

echo === Testing Commands with Input Redirection ===
echo.

echo [TEST 01] help command...
(
    echo l
    echo help
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"Available commands" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 02] login command...
(
    echo l
    echo login 2116 dddd
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"logged in" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 03] mkdir command...
(
    echo l
    echo login 2116 dddd
    echo mkdir testdir003
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"created" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 04] ls command...
(
    echo l
    echo login 2116 dddd
    echo ls
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"CURRENT DIRECTORY" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 05] create command...
(
    echo l
    echo login 2116 dddd
    echo create testfile005.txt
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"created" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 06] open command...
(
    echo l
    echo login 2116 dddd
    echo create testopen006.txt
    echo open testopen006.txt w
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"fd=" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 07] write command...
(
    echo l
    echo login 2116 dddd
    echo create testwrite007.txt
    echo open testwrite007.txt w
    echo write 0 testcontent
    echo close 0
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"Wrote" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 08] read command...
(
    echo l
    echo login 2116 dddd
    echo create testread008.txt
    echo open testread008.txt w
    echo write 0 readtestdata
    echo close 0
    echo open testread008.txt r
    echo read 0 50
    echo close 0
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"readtestdata" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 09] cat command...
(
    echo l
    echo login 2116 dddd
    echo create testcat009.txt
    echo open testcat009.txt w
    echo write 0 catcontentdata
    echo close 0
    echo cat testcat009.txt
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"catcontentdata" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 10] delete command...
(
    echo l
    echo login 2116 dddd
    echo create testdel010.txt
    echo delete testdel010.txt
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"deleted" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 11] logout command...
(
    echo l
    echo login 2116 dddd
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"logged out" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 12] user command...
(
    echo l
    echo login 2116 dddd
    echo user
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"UID" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 13] pwd command...
(
    echo l
    echo login 2116 dddd
    echo pwd
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"Current path" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 14] cd command...
(
    echo l
    echo login 2116 dddd
    echo mkdir testcd014
    echo cd testcd014
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"does not existed" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo [TEST 15] close command...
(
    echo l
    echo login 2116 dddd
    echo create testclose015.txt
    echo open testclose015.txt w
    echo close 0
    echo logout
    echo exit
) > "%TEMP_CMDS%"
"%MAIN_EXE%" < "%TEMP_CMDS%" > "%TEMP_OUT%" 2>&1
findstr /C:"closed" "%TEMP_OUT%" > nul
if errorlevel 1 (echo [FAIL]) else (echo [PASS])

echo.
echo ================================================
echo   All Tests Complete
echo ================================================
pause
