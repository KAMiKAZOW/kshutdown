call C:\Qt\4.3.1\bin\qtvars.bat
cd src
qmake -config release
call make clean

call make
if not %errorlevel% == 0 goto quit

cd ..

"C:\Program Files\NSIS\makensis.exe" kshutdown.nsi
if not %errorlevel% == 0 goto quit
kshutdown-2.0alpha3-win32.exe

:quit