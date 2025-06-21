@ECHO OFF
for /R %%a in (".") do (
cd "%%a"
for /R %%b in (".") do (
cd "%%b"

move *Another7*.cs2 a7.cs2
move *Another14*.cs2 a14.cs2
move *Light14*.cs2 n14.cs2
move *Light7*.cs2 n7.cs2
move *7Keys*.cs2 h7.cs2
move *14Keys*.cs2 h14.cs2

move *Another7*.cs a7.cs
move *Another14*.cs a14.cs
move *Light14*.cs n14.cs
move *Light7*.cs n7.cs
move *7Keys*.cs h7.cs
move *14Keys*.cs h14.cs

move *^ * 01.wav

@ECHO ON

D:\bms\cs2ac >> ..\..\log.txt
@ECHO OFF
D:\bms\created2dx_cs2emp
@ECHO OFF
cd ..\..
)
)
