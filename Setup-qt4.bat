call C:\Qt\4.4.3\bin\qtvars.bat
cd src

rem goto skip_portable
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

:skip_portable

rem normal version
qmake -config release
call make clean
call make
if not %errorlevel% == 0 goto quit

cd ..

"C:\Program Files\NSIS\makensis.exe" kshutdown.nsi
if not %errorlevel% == 0 goto quit
kshutdown-2.0beta7-win32.exe

copy C:\Qt\4.4.3\bin\mingwm10.dll kshutdown-portable

:quit
