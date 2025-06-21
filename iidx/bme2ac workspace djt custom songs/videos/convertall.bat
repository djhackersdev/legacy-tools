@ECHO OFF
for /R %%a in (".") do (
cd "%%a"
for /R %%b in (".") do (
cd "%%b"
@ECHO ON
C:\bms\convertmovie h7.bme
@ECHO OFF
cd ..\..
)
)
