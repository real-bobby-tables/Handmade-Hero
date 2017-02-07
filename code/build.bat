@echo off
call shell
IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl -Zi d:\code\win32_handmade.cpp user32.lib Gdi32.lib
popd