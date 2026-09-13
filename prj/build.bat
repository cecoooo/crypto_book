@echo off
setlocal

set BUILD_DIR=build

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

cmake -S . -B "%BUILD_DIR%" -DBUILD_BENCHMARKS=ON

if errorlevel 1 (
    echo.
    echo CMake configuration failed.
    exit /b 1
)

cmake --build "%BUILD_DIR%" --config Release

if errorlevel 1 (
    echo.
    echo Build failed.
    exit /b 1
)

echo.
echo Build completed successfully.
echo Application: %BUILD_DIR%\bin\Release\order_book_app.exe
echo Benchmark: %BUILD_DIR%\bin\Release\order_book_benchmark.exe

endlocal