@ECHO OFF

mkdir build
chdir build
cmake -Wdev -G "Visual Studio 14" ../
pause
