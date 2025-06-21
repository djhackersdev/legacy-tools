FOR %%i IN (1 2 3 4 5 6 7 8 9) DO (
mkdir 0%%i
move B_%%i??.tga 0%%i
cd 0%%i
mkdir in_0%%i
..\..\packgcz in_0%%i *.tga
move in_0%%i ..
cd ..
)
FOR %%i IN (10 11 12 13 14 15) DO (
mkdir %%i
move B_%%i??.tga %%i
cd %%i
mkdir in_%%i
..\..\packgcz in_%%i *.tga
move in_%%i ..
cd ..
)
mkdir mdata
..\packgcz mdata *.tga