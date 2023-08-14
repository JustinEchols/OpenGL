@echo off

echo --------
echo --------

set wildcard=*.h *.cpp

echo TODOS FOUND:
findstr -s -n -i -l "TODO" %wildcard%


echo --------
echo --------

