command /c rem
mkdir "%~dps0old"
move "%~dps0*.wav" "%~dps0old"
for %%f in ("%~dps0old\*.wav") do (
	rename "%%~sf" "temp.wav"
	sox -G -S "%%~spftemp.wav" -e ms-adpcm "%%~spfout.wav"
	move "%%~spfout.wav" "%~dps0%%~nf.wav"
	rename "%%~spftemp.wav" "%%~nf.wav"
)
for %%f in ("%~dps0*.ogg") do (
	rename "%%~sf" "temp.ogg"
	sox -G -S "%%~spftemp.ogg" -e ms-adpcm "%%~spfout.wav"
	rename "%%~spfout.wav" "%%~nf.wav"
	rename "%%~spftemp.ogg" "%%~nf.ogg"
)