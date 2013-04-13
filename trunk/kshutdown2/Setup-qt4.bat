call C:\Qt\4.8.4\bin\qtvars.bat
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
kshutdown-3.0-win32.exe

:skip_normal
copy README.html kshutdown-portable
copy C:\mingw\bin\libgcc_s_dw2-1.dll kshutdown-portable
copy C:\mingw\bin\libstdc++-6.dll kshutdown-portable
copy C:\mingw\bin\mingwm10.dll kshutdown-portable
copy C:\Qt\4.8.4\bin\QtCore4.dll kshutdown-portable
copy C:\Qt\4.8.4\bin\QtGui4.dll kshutdown-portable

:quit

echo "DONE"
