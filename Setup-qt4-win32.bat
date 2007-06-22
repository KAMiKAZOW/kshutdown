call C:\Qt\4.3.0\bin\qtvars.bat
cd src
qmake
call make clean
call make
cd ..
REM FIXME: check last error
REM "C:\Program Files\NSIS\makensis.exe" kshutdown.nsi
