@echo off
subst d: C:\Users\brand\Documents\HandmadeHero
cd C:\Users\brand\Documents\HandmadeHero
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
set path=d:\misc;%path%
cd C:\Users\brand\Documents\HandmadeHero\code