mkdir old
move *.wav old
for %%s in (old\*.wav) do sox "%%s" -S -a "%%~ns%%~xs" rate -v 44k
