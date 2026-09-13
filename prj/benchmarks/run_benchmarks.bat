@echo off
setlocal

set ROOT=%~dp0..
set BUILD=%ROOT%\build-benchmark
set LOG=%ROOT%\benchmark_logs\performance.txt

cmake -S "%ROOT%" -B "%BUILD%" -DBUILD_BENCHMARKS=ON
if errorlevel 1 exit /b 1

cmake --build "%BUILD%" --config Release --target order_book_benchmark
if errorlevel 1 exit /b 1

if exist "%LOG%" del "%LOG%"

for %%B in (131072 262144 524288) do (
    "%BUILD%\bin\Release\order_book_benchmark.exe" --mode data --scenario mixed --levels 250000 --updates 1000000 --runs 5 --buffer %%B --log "%LOG%"
    if errorlevel 1 exit /b 1
)

"%BUILD%\bin\Release\order_book_benchmark.exe" --mode data --scenario snapshot --levels 1000000 --runs 5 --buffer 262144 --log "%LOG%"
if errorlevel 1 exit /b 1

echo.
echo Results: %LOG%
endlocal
