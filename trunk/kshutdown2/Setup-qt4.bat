call C:\Qt\4.3.2\bin\qtvars.bat
cd src

rem portable version
rem FIXME: omg, no quotes here?
echo DEFINES += KS_PORTABLE>portable.pri
qmake -config release
call make clean
call make
if not %errorlevel% == 0 goto quit
mkdir ..\kshutdown-portable
copy release\kshutdown.exe ..\kshutdown-portable
del portable.pri

rem normal version
qmake -config release
call make clean
call make
if not %errorlevel% == 0 goto quit

cd ..

"C:\Program Files\NSIS\makensis.exe" kshutdown.nsi
if not %errorlevel% == 0 goto quit
kshutdown-2.0alpha5-win32.exe

:quit