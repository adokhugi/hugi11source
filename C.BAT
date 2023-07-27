@echo off
cls

if "%1" == "c" goto full

rem tasm32 song.asm
rem tasm32 pics.asm
wpp386 vesavbe.c -fpi87
wpp386 hugi.cpp -fpi87
wlink file hugi,vesavbe,song,pics,texts name hugi library ims.lib,binfile.lib

goto ende

:full

:song- und textlength mÅssen manuell in const*.h eingetragen werden!

tasm32 song.asm
tasm32 pics.asm
tasm32 texts.asm

wpp386 vesavbe.c -fpi87
wpp386 hugi.cpp -fpi87
wlink file hugi,vesavbe,song,pics,texts name hugi library ims.lib,binfile.lib

pmwlite hugi.exe

:ende