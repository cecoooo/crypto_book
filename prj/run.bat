@echo off
setlocal

set EXECUTABLE=build\bin\Release\order_book_app.exe

if not exist "%EXECUTABLE%" (
    echo Executable not found: %EXECUTABLE%
    echo Run build.bat first.
    exit /b 1
)

"%EXECUTABLE%"

endlocal