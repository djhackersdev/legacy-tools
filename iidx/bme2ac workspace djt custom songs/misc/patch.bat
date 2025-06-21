@ECHO OFF
for /R %%a in (".") do (
cd "%%a"
@ECHO ON

echo "%%a"
..\fixtimingwindows.exe *.1
@ECHO OFF
cd ..
)