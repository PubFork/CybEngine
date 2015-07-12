@echo off

echo Removing build directory...
rmdir /S /Q build 2> nul

echo Removing bin directory...
rmdir /S /Q bin 2> nul

echo Done.