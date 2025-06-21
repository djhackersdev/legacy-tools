@ECHO OFF
for /r %%a in (".") do (
cd "%%a"
rem for /r %%b in (".") do (
rem cd "%%b"

echo %%a
 @ECHO ON

 ..\..\timebase_59 *.1 >> ..\..\log.txt

 for %%c in (*.2dx) do (
 ..\..\oldac2dx2newac2dx %%c >> ..\..\log.txt
 )

@ECHO OFF
cd ..\..
)
)
