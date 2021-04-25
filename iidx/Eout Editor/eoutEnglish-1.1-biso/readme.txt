EditOut -2chan Sucks Edition-
v1.1 for bemaniso
--------------------------------------
Files found on 2ch by musicboy
Translations done by Soveryshiny
Translations Added to code by Anthony
Program Recompiled by infamouspat
Eout Encrypter/Decrypter by Tau
Testing and mdata provided by Raoh
--------------------------------------
Included in this package:

mdata (DIR)		- GOLD mdata JPGs
split (DIR)		- Required Files.
10-12eout.dec		- 10th - HS eout merged
13eout.dec		- DistorteD eout
14eout			- GOLD eout
decrypt.exe		- eout.bin decrypter
encrypt.exe		- eout.bin encrypter
eoutEdit.exe		- eout Editor execulatable
eoutEdit.jar		- eout Editor jar file
readme.txt		- readme
readme-2ch.rtf		- original 2chan readme
swt-win32-3236.dll	- dll file required for eout Editor
--------------------------------------

Instructions:

--------------------------------------
Decompressing the eout.bin file
--------------------------------------
1) Place the eout.bin from the game you wish to use
(found in data/info) into the eoutEnglish folder.

2) In your command line navigate to your eoutEnglish
folder and run "decrypt eout.bin eout.dec". You will
now have a decrypted eout file.

--------------------------------------
Opening the eout file
--------------------------------------
1) In eoutEdit change your CSV Settings to the game you
will be editing. THIS MUST BE DONE BEFORE STEP 2 OR
YOUR SAVED EOUT WILL BE CORRUPT.

2) Open your eout.dec in eoutEdit.

--------------------------------------
Adding Data to your eout
--------------------------------------
Adding data to your eout file is rather easy. Find the number
of the song you wish to add to your name and click the number.
It will ask you if you wish to add it. Click yes and begin
entering the data. The best way to find the data you need is
to run a second instance of eoutEdit and load in one of the
included eout.dec files and get the data from that file.
Remember you must click save to every individual page of data
to properly add the song.

**NOTE** If you try to add a song that does NOT have graphics in
the current game's mdata then you will get HDD errors on game
boot. I have included a folder of GOLD's mdata in JPG format
for those of you working with GOLD.

After adding a song into your list, copy it's sd_data, movie (if
available) and overlays (if available) to the new game directory.

Examples!

To add Car of Your Dreams (movie) to GOLD you must copy:
1)
1106 (DIR) from the previous game's 'data\sd_data' to your
GOLD's 'data\sd_data'
2)
1106.4 (DIR) from the previous game's 'data\movie' to your
GOLD's 'data\movie'

To add Samba de Janeiro (overlay) to GOLD you must copy:
1)
1304 (DIR) from the previous game's 'data\sd_data' to your
GOLD's 'data\sd_data'
2)
1304 (DIR) from the previous game's 'data\graph\sys' to your
GOLD's 'data\graph\sys'

When in doubt of what you need to move, check both data\movie and
data\graph\sys for files with the same number as the song.

Once you are done saving songs just click 'File->Save' to resave the
eout.dec file.

--------------------------------------
Recompressing the eout.bin file
--------------------------------------
1) In your command line navigate to your eoutEnglish
folder and run "encrypt eout.dec eout.bin". You will
now have an encrypted eout file.

2) Place the eout.bin from your eoutEnglish folder
back into the 'data/info' folder of the game.

REMEMBER TO MAKE BACKUPS!!! This program does not have a delete
song option. So if a song you add does not work you will need to
go to your backups. If you don't make backups, the most you can
do is blank song info out and end up with songwheel cluster.

Thats all you need to do.

For discussion on this please refer to:
http://bemaniso.ws/forums.php?action=viewtopic&topicid=9206


-Anthony