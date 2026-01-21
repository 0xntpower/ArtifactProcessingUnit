@echo off
echo Building Artifact Processing Unit...
echo.

REM Find MSBuild
set MSBUILD=""
for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set MSBUILD="%%i\MSBuild\Current\Bin\MSBuild.exe"
)

if not exist %MSBUILD% (
    echo ERROR: MSBuild not found. Please install Visual Studio 2022.
    exit /b 1
)

echo Found MSBuild: %MSBUILD%
echo.

REM Install vcpkg dependencies if needed
if not exist "vcpkg_installed" (
    echo Installing vcpkg dependencies...
    vcpkg install
    echo.
)

REM Build the solution
echo Building solution...
%MSBUILD% ArtifactProcessingUnit.sln /p:Configuration=Debug /p:Platform=x64 /v:minimal /m

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo BUILD FAILED!
    exit /b %ERRORLEVEL%
)

echo.
echo BUILD SUCCESSFUL!
echo Executable: src\ArtifactProcessingUnit\x64\Debug\ArtifactProcessingUnit.exe
