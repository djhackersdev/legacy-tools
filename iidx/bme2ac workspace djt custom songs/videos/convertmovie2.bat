for %%s in (*.bmp) do convert "%%s" -resize 256x256! -type TrueColor "%%s"
for %%s in (*.png) do (
convert "%%s" -resize 256x256! -type TrueColor "%%~ns.bmp"
move "%%~ns.bmp" "%%s"
)
for %%s in (*.jpg) do (
convert "%%s" -resize 256x256! -type TrueColor "%%~ns.bmp"
move "%%~ns.bmp" "%%s"
)
copy C:\mplayer\BLACK.bmp .
C:\mplayer\bme2picseq "%1"
@ECHO OFF
C:\mplayer\mencoder mf://BMEIMG*.bmp -mf fps=30 -ovc lavc -of mpeg -mpegopts format=mpeg2video:tsaf -vf crop=256:218:0:38,scale=304:416 -lavcopts vcodec=mpeg2video:vrc_buf_size=1835:vrc_maxrate=9000:vbitrate=6000:keyint=15:trell:mbd=2:dc=10:vstrict=0:vpass=1:aspect=1/1 -o output.mpg
C:\mplayer\mencoder mf://BMEIMG*.bmp -mf fps=30 -ovc lavc -of mpeg -mpegopts format=mpeg2video:tsaf -vf crop=256:218:0:38,scale=304:416 -lavcopts vcodec=mpeg2video:vrc_buf_size=1835:vrc_maxrate=3500:vbitrate=2500:keyint=15:trell:mbd=2:dc=10:vstrict=0:vpass=2:aspect=1/1 -o output.mpg
del BMEIMG*
del BLACK.bmp
@ECHO ON
