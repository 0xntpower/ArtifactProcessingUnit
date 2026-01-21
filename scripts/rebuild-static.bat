@echo off
echo ====================================
echo Rebuilding with Static Libraries
echo ====================================
echo.

echo Step 1: Removing old x64-windows packages...
if exist "vcpkg_installed\x64-windows" (
    rd /s /q "vcpkg_installed\x64-windows"
)

echo.
echo Step 2: Installing x64-windows-static packages...
echo This may take 20-45 minutes on first run...
echo.

where vcpkg >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: vcpkg not found in PATH
    echo Please add vcpkg to your PATH or run this from vcpkg directory
    pause
    exit /b 1
)

vcpkg install --triplet=x64-windows-static

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: vcpkg install failed
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo Step 3: Finding MSBuild...

set MSBUILD=""
for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set MSBUILD="%%i\MSBuild\Current\Bin\MSBuild.exe"
)

if not exist %MSBUILD% (
    echo ERROR: MSBuild not found. Please install Visual Studio 2022.
    pause
    exit /b 1
)

echo Found MSBuild: %MSBUILD%
echo.

echo Step 4: Cleaning previous build...
%MSBUILD% ArtifactProcessingUnit.sln /t:Clean /p:Configuration=Debug /p:Platform=x64 /v:minimal

echo.
echo Step 5: Building solution...
%MSBUILD% ArtifactProcessingUnit.sln /p:Configuration=Debug /p:Platform=x64 /v:minimal /m

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ====================================
    echo BUILD FAILED!
    echo ====================================
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ====================================
echo BUILD SUCCESSFUL!
echo ====================================
echo.
echo Executable: src\ArtifactProcessingUnit\x64\Debug\ArtifactProcessingUnit.exe
echo.
echo No DLLs needed - all libraries are statically linked.
echo.
pause
