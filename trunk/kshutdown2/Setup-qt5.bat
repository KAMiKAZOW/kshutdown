set KS_APP_VERSION=%1
set KS_QT_VERSION=%2
set KS_COMMAND=%3

if not defined KS_APP_VERSION (
	echo Error: Missing application version option
	echo Usage: ./Setup-wine.sh
	pause
	exit /B 1
)

if not defined KS_QT_VERSION (
	echo Error: Missing Qt version option
	echo Usage: ./Setup-wine.sh
	pause
	exit /B 1
)

if not defined KS_COMMAND (
	echo Error: Missing command option
	echo Usage: ./Setup-wine.sh
	pause
	exit /B 1
)

if "%KS_COMMAND%" == "setup" (
	goto nsis
)

set KS_QT_BIN=C:\Qt\Qt%KS_QT_VERSION%\%KS_QT_VERSION%\mingw53_32\bin
pushd .
call "%KS_QT_BIN%\qtenv2.bat"
popd

if "%KS_COMMAND%" == "test" (
	pushd src
	mingw32-make.exe -j2 && release\kshutdown.exe
	pause
	popd
	goto quit
)

rem pause

rem TEST:
rem cd ..
rem goto nsis
rem goto skip_portable
rem goto zip

rem ==== portable version ====

pushd src
echo DEFINES += KS_PORTABLE>portable.pri
qmake -config release
mingw32-make.exe clean
mingw32-make.exe -j2
pause
if not %errorlevel% == 0 goto quit
mkdir ..\kshutdown-portable
copy release\kshutdown.exe ..\kshutdown-portable
del portable.pri
popd

rem TEST:
rem cd ..
rem goto zip

:skip_portable

rem ==== normal version ====

pushd src
qmake -config release
mingw32-make.exe clean
mingw32-make.exe -j2
pause
if not %errorlevel% == 0 goto quit
popd

rem ==== portable version package ====

:zip
copy README.html kshutdown-portable
pushd kshutdown-portable
%KS_QT_BIN%\windeployqt.exe -no-angle -no-opengl-sw kshutdown.exe
del Qt5Svg.dll
rmdir /Q /S "iconengines"
rmdir /Q /S "imageformats"
popd

rem ==== installer package ====

:nsis
if exist "%ProgramFiles(x86)%\NSIS\makensis.exe" (
	"%ProgramFiles(x86)%\NSIS\makensis.exe" /DAPP_VERSION=%KS_APP_VERSION% kshutdown.nsi
) else (
	"%ProgramFiles%\NSIS\makensis.exe" /DAPP_VERSION=%KS_APP_VERSION% kshutdown.nsi
)
pause
if not %errorlevel% == 0 goto quit
kshutdown-%KS_APP_VERSION%-win32.exe

:quit
echo DONE
