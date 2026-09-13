@echo off
setlocal

set ROOT=%~dp0..
set BUILD=%ROOT%\build

cmake -S "%ROOT%" -B "%BUILD%" -DBUILD_TESTING=ON
if errorlevel 1 exit /b 1

cmake --build "%BUILD%" --config Release --target order_book_tests
if errorlevel 1 exit /b 1

ctest --test-dir "%BUILD%" -C Release --output-on-failure
if errorlevel 1 exit /b 1

"%ROOT%\build\bin\Release\order_book_tests.exe"
if errorlevel 1 exit /b 1

endlocal