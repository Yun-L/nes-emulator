@echo off
pushd "%~dp0"
mkdir .\build
pushd .\build
cl /Zi /Wall ..\src\main.cpp
popd
popd
