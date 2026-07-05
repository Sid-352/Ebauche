@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "ROOT=%~dp0"
set "CONFIG=Debug"
set "NO_PAUSE=0"

for %%A in (%*) do (
    if /I "%%~A"=="Release" set "CONFIG=Release"
    if /I "%%~A"=="Debug" set "CONFIG=Debug"
    if /I "%%~A"=="--no-pause" set "NO_PAUSE=1"
)

where cl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [build] cl.exe not found, trying to load MSVC environment...

    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
    ) else (
        echo [build] Could not locate vcvars64.bat for VS2022.
        echo [build] Install Visual Studio Build Tools 2022 and C++ workload.
        goto :fail
    )
)

where cl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [build] MSVC environment setup failed. cl.exe is still unavailable.
    goto :fail
)

set "OBJ_DIR=%ROOT%build\obj\%CONFIG%"
set "BIN_DIR=%ROOT%build\bin\%CONFIG%"
set "EXE_PATH=%BIN_DIR%\Ebauche.exe"

if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"

set "SRC_LIST="
for /R "%ROOT%src" %%F in (*.cpp) do (
    set "SRC_LIST=!SRC_LIST! "%%~fF""
)

if "%SRC_LIST%"=="" (
    echo [build] No C++ source files found under src\
    echo [build] Add .cpp files to src\ and run build.bat again.
    goto :fail
)

set "COMMON_FLAGS=/nologo /std:c++20 /EHsc /W4 /permissive- /Zc:__cplusplus /DUNICODE /D_UNICODE /I"%ROOT%src" /I"%ROOT%third_party\raylib\include" /I"%ROOT%third_party\imgui" /I"%ROOT%third_party\rlimgui""
set "LINK_FLAGS=/nologo /MACHINE:X64 /INCREMENTAL:NO /SUBSYSTEM:WINDOWS /LIBPATH:"%ROOT%third_party\raylib\lib" /OUT:"%EXE_PATH%" raylib.lib user32.lib gdi32.lib winmm.lib shell32.lib ole32.lib comdlg32.lib advapi32.lib"

if /I "%CONFIG%"=="Release" (
    set "CFLAGS=/O2 /MD /DNDEBUG"
    set "LFLAGS="
) else (
    set "CFLAGS=/Od /Zi /MDd /DDEBUG"
    set "LFLAGS=/DEBUG:FULL"
)

echo [build] Configuration: %CONFIG%
echo [build] Output: %EXE_PATH%

cl %COMMON_FLAGS% %CFLAGS% /Fo"%OBJ_DIR%\\" %SRC_LIST% /link %LINK_FLAGS% %LFLAGS%
if %ERRORLEVEL% NEQ 0 goto :fail

echo.
echo [build] SUCCESS
goto :done

:fail
echo.
echo [build] FAILED

:done
echo.
if "%NO_PAUSE%"=="1" goto :eof
pause
