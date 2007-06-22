call C:\Qt\4.3.0\bin\qtvars.bat
cd src
qmake
make clean
make
cd ..
REM FIXME: check last error
C:\Program Files\NSIS\makensis.exe kshutdown.nsi
