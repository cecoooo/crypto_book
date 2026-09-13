@echo off
setlocal

if "%~2"=="" (
    echo Usage: run_file_benchmark.bat SNAPSHOT_FILE UPDATES_FILE
    exit /b 1
)

set ROOT=%~dp0..
set BUILD=%ROOT%\build-benchmark
set LOG=%ROOT%\benchmark_logs\file_performance.txt

cmake -S "%ROOT%" -B "%BUILD%" -DBUILD_BENCHMARKS=ON
if errorlevel 1 exit /b 1

cmake --build "%BUILD%" --config Release --target order_book_benchmark
if errorlevel 1 exit /b 1

if exist "%LOG%" del "%LOG%"

"%BUILD%\bin\Release\order_book_benchmark.exe" --mode files --snapshot "%~1" --updates-file "%~2" --runs 5 --log "%LOG%"
if errorlevel 1 exit /b 1

echo.
echo Results: %LOG%
endlocal
