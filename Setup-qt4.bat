call C:\Qt\2010.05\bin\qtenv.bat
cd src

rem goto skip_portable
rem portable version
echo DEFINES += KS_PORTABLE>portable.pri
qmake -config release
mingw32-make.exe clean
mingw32-make.exe
if not %errorlevel% == 0 goto quit
mkdir ..\kshutdown-portable
copy release\kshutdown.exe ..\kshutdown-portable
del portable.pri

:skip_portable

rem normal version
qmake -config release
mingw32-make.exe clean
mingw32-make.exe
if not %errorlevel% == 0 goto quit

cd ..

"C:\Program Files\NSIS\makensis.exe" kshutdown.nsi
if not %errorlevel% == 0 goto quit
kshutdown-2.1beta-win32.exe

copy C:\Qt\2010.05\mingw\bin\libgcc_s_dw2-1.dll kshutdown-portable
copy C:\Qt\2010.05\mingw\bin\mingwm10.dll kshutdown-portable
copy C:\Qt\2010.05\qt\bin\QtCore4.dll kshutdown-portable
copy C:\Qt\2010.05\qt\bin\QtGui4.dll kshutdown-portable

:quit

echo "DONE"
