mkdir old
move *.ogg old
for %%s in (old\*.ogg) do sox "%%s" -S -a "%%~ns.wav"
