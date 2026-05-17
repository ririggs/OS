@echo off
echo === Building Employee Pipe Server ^& Client ===
echo.
echo Choose compiler:
echo   1. MinGW / GCC (g++)
echo   2. MSVC  (cl - Developer Command Prompt)
echo.

setlocal

where g++ >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [GCC] Building server...
    g++ -std=c++11 -o server.exe server.cpp -lws2_32
    if %ERRORLEVEL% NEQ 0 goto :fail

    echo [GCC] Building client...
    g++ -std=c++11 -o client.exe client.cpp -lws2_32
    if %ERRORLEVEL% NEQ 0 goto :fail

    echo.
    echo === Build successful ===
    echo Run: server.exe
    goto :end
)

where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [MSVC] Building server...
    cl /EHsc /Fe:server.exe server.cpp
    if %ERRORLEVEL% NEQ 0 goto :fail

    echo [MSVC] Building client...
    cl /EHsc /Fe:client.exe client.cpp
    if %ERRORLEVEL% NEQ 0 goto :fail

    echo.
    echo === Build successful ===
    echo Run: server.exe
    goto :end
)

echo No compiler found (g++ or cl). Install MinGW or run from Developer Command Prompt.
goto :end

:fail
echo.
echo === Build FAILED ===

:end
endlocal
pause
