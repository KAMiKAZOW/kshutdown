
rem Usage: wineconsole tools/test-wine.bat

call C:\Qt\4.8.6\bin\qtvars.bat
cd src
qmake -config release
mingw32-make.exe
cd ..
rem "C:\Program Files\NSIS\makensis.exe" kshutdown.nsi
src\release\kshutdown-qt.exe
