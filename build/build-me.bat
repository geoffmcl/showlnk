@setlocal

@set DOTINST=0
@set DOINSTALL=1

@set TMPDIR=F:
@REM set TMPRT=%TMPDIR%\FG\18
@set TMPRT=..
@set TMPVER=1
@set TMPPRJ=showlnk
@set TMPSRC=%TMPRT%
@set TMPBGN=%TIME%
@set TMPINS=C:/MDOS
@set TMPCM=%TMPSRC%\CMakeLists.txt
@set DOPAUSE=pause

@REM call chkmsvc %TMPPRJ% 

@if EXIST build-cmake.bat (
@call build-cmake
)

@if NOT EXIST %TMPCM% goto NOCM

@set TMPLOG=bldlog-1.txt
@set TMPOPTS=-DCMAKE_INSTALL_PREFIX=%TMPINS%
:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPOPTS=%TMPOPTS% %1
@shift
@goto RPT
:GOTCMD

@echo Build %DATE% %TIME% > %TMPLOG%
@echo Build source %TMPSRC%... all output to build log %TMPLOG%
@echo Build source %TMPSRC%... all output to build log %TMPLOG% >> %TMPLOG%

cmake %TMPSRC% %TMPOPTS% >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR1

cmake --build . --config Debug >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR2

cmake --build . --config Release >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR3
:DONEREL

@fa4 "***" %TMPLOG%
@call elapsed %TMPBGN%
@echo Appears a successful build... see %TMPLOG%

@if "%DOTINST%x" == "0x" (
@echo Skipping 'temp' install for now...
@goto DNTINST
)

@echo Building installation zips... moment...
@REM If paths given as ${CMAKE_INSTALL_PREFIX}
@call build-zipsf Debug
@call build-zipsf Release
@REM If full paths used in install
@REM call build-zipsf2 Debug
@REM call build-zipsf2 Release
@echo Done installation zips...
:DNTINST

@if "%DOINSTALL%x" == "0x" (
@echo Skipping install for now...
@goto END
)
@echo Continue with install? Only Ctrl+c aborts...
@%DOPAUSE%

@goto DNDBGINST
cmake --build . --config Debug  --target INSTALL >> %TMPLOG% 2>&1
@if EXIST install_manifest.txt (
@copy install_manifest.txt install_manifest_dbg.txt >nul
@echo. >> %TMPINS%\installed.txt
@echo %TMPPRJ% Debug install %DATE% %TIME% >> %TMPINS%\installed.txt
@type install_manifest.txt >> %TMPINS%\installed.txt
)
:DNDBGINST

cmake --build . --config Release  --target INSTALL >> %TMPLOG% 2>&1
@if EXIST install_manifest.txt (
@copy install_manifest.txt install_manifest_rel.txt >nul
@echo. >> %TMPINS%\installed.txt
@echo %TMPPRJ% Release install %DATE% %TIME% >> %TMPINS%\installed.txt
@type install_manifest.txt >> %TMPINS%\installed.txt
)

@echo.
@fa4 " -- " %TMPLOG%
@echo.

@call elapsed %TMPBGN%
@echo All done... see %TMPLOG%

@goto END

:NOCM
@echo Error: Can NOT locate %TMPCM%
@goto ISERR

:ERR1
@echo cmake configuration or generations ERROR
@goto ISERR

:ERR2
@echo ERROR: Cmake build Debug FAILED!
@goto ISERR

:ERR3
@fa4 "mt.exe : general error c101008d:" %TMPLOG% >nul
@if ERRORLEVEL 1 goto ERR33
:ERR34
@echo ERROR: Cmake build Release FAILED!
@goto ISERR
:ERR33
@echo Try again due to this STUPID STUPID STUPID error
@echo Try again due to this STUPID STUPID STUPID error >>%TMPLOG%
cmake --build . --config Release >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR34
@goto DONEREL

:ISERR
@echo See %TMPLOG% for details...
@endlocal
@exit /b 1

:END
@endlocal
@exit /b 0

@REM eof
