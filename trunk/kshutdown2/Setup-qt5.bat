set KS_QT_VERSION=5.11.0
set KS_QT_BIN=C:\Qt\Qt%KS_QT_VERSION%\%KS_QT_VERSION%\mingw53_32\bin
pushd .
call "%KS_QT_BIN%\qtenv2.bat"
popd
cd src

rem TEST:
rem cd ..
rem goto nsis
rem goto skip_portable
rem goto zip

rem ==== portable version ====

echo DEFINES += KS_PORTABLE>portable.pri
qmake -config release
mingw32-make.exe clean
mingw32-make.exe -j2
if not %errorlevel% == 0 goto quit
mkdir ..\kshutdown-portable
copy release\kshutdown-qt.exe ..\kshutdown-portable\kshutdown.exe
del portable.pri

rem TEST:
rem cd ..
rem goto zip

:skip_portable

rem ==== normal version ====

qmake -config release
mingw32-make.exe clean
mingw32-make.exe -j2
if not %errorlevel% == 0 goto quit

rem ==== installer package ====

cd ..

:nsis
if exist "%ProgramFiles(x86)%\NSIS\makensis.exe" (
	"%ProgramFiles(x86)%\NSIS\makensis.exe" kshutdown.nsi
) else (
	"%ProgramFiles%\NSIS\makensis.exe" kshutdown.nsi
)
if not %errorlevel% == 0 goto quit
kshutdown-5.0beta1-win32.exe

rem ==== portable version package ====

:zip
copy README.html kshutdown-portable

copy "%KS_QT_BIN%\libgcc_s_dw2-1.dll" kshutdown-portable
copy "%KS_QT_BIN%\libwinpthread-1.dll" kshutdown-portable
copy "%KS_QT_BIN%\libstdc++-6.dll" kshutdown-portable

copy "%KS_QT_BIN%\Qt5Core.dll" kshutdown-portable
copy "%KS_QT_BIN%\Qt5Gui.dll" kshutdown-portable
copy "%KS_QT_BIN%\Qt5Widgets.dll" kshutdown-portable
copy "%KS_QT_BIN%\..\plugins\platforms\qwindows.dll" kshutdown-portable

:quit
echo "DONE"
