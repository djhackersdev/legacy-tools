mkdir old
move *.wav old
for %%s in (old\*.wav) do sox "%%s" -S -a -c 1  "%%~ns%%~xs"  rate -v 21k
