call C:\Qt\4.3.1\bin\qtvars.bat
cd src
qmake
call make clean
call make
cd ..
REM FIXME: check last error
"C:\Program Files\NSIS\makensis.exe" kshutdown.nsi
kshutdown-2.0alpha3-win32.exe