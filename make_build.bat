@ECHO OFF

mkdir build
chdir build
cmake -Wdev -G "Visual Studio 12" ../
pause
