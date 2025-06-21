mogrify -resize 33%x33% L_*.png A_*.png G_*.png
mogrify -resize 85%x85% T_*.png
mogrify -format tga -type TrueColorMatte *.png