
rem Usage: wineconsole tools/test-wine.bat

call C:\Qt\4.8.7\bin\qtvars.bat
cd src
qmake -config release
mingw32-make.exe -j2
pause
cd ..

rem Installer test:
rem "%ProgramFiles%\NSIS\makensis.exe" kshutdown.nsi
rem kshutdown-4.99-beta-win32.exe

src\release\kshutdown-qt.exe
