set KS_QT_VERSION=5.11.2
set KS_QT_BIN=C:\Qt\Qt%KS_QT_VERSION%\%KS_QT_VERSION%\mingw53_32\bin
pushd .
call "%KS_QT_BIN%\qtenv2.bat"
popd

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
	"%ProgramFiles(x86)%\NSIS\makensis.exe" kshutdown.nsi
) else (
	"%ProgramFiles%\NSIS\makensis.exe" kshutdown.nsi
)
pause
if not %errorlevel% == 0 goto quit
kshutdown-5.0-win32.exe

:quit
echo "DONE"
