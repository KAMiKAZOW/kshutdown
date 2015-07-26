set KS_QT_BIN="C:\Qt\4.8.6\bin"
call %KS_QT_BIN%\qtvars.bat
cd src

rem goto skip_portable
rem portable version
echo DEFINES += KS_PORTABLE>portable.pri
qmake -config release
mingw32-make.exe clean
mingw32-make.exe
if not %errorlevel% == 0 goto quit
mkdir ..\kshutdown-portable
copy release\kshutdown-qt.exe ..\kshutdown-portable\kshutdown.exe
del portable.pri

rem cd ..
rem goto skip_normal

:skip_portable

rem normal version
qmake -config release
mingw32-make.exe clean
mingw32-make.exe
if not %errorlevel% == 0 goto quit

cd ..

"C:\Program Files\NSIS\makensis.exe" kshutdown.nsi
if not %errorlevel% == 0 goto quit
kshutdown-3.3.2beta-win32.exe

:skip_normal
copy README.html kshutdown-portable
copy C:\mingw32\bin\libgcc_s_dw2-1.dll kshutdown-portable
copy "C:\mingw32\bin\libstdc++-6.dll" kshutdown-portable
copy C:\mingw32\bin\libwinpthread-1.dll kshutdown-portable
copy %KS_QT_BIN%\QtCore4.dll kshutdown-portable
copy %KS_QT_BIN%\QtGui4.dll kshutdown-portable

:quit

echo "DONE"
