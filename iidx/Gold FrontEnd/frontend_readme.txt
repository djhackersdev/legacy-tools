IIDXGOLD Frontend v1.3 - by PoP
-==-=-=-=-=-=-=-=-=-=-=-=-
Frontend for IIDX GOLD AC.

-! IF YOU'RE INSTALLING OVER AN OLD VERSION !-
Play one game with all default settings (frame, BGM etc.)
This will reverse all of the damage the old version hath
wrought on thou's files.  Then delete settings.dat, 
the FrontendFiles folder and the old .EXE.

Extract Frontend.exe, FrontendFiles and copy_frontendfiles.bat
to the root of GOLD's DIR (with data and 2007022101).
Run copy_frontendfiles.bat to fill FrontendFiles,
which is just copies some audio files that can be
replaced by the frontend.  It's a bit safer but it's
more wasteful than careful renaming, but I wasn't 
originally building this for anyone but myself anyway.
I've taken more care with the newer releases though,
simply because I'm finding the program more useful now
that I figured out some of the 2DXWAVE options.
  
Also, you need to make three different input.dat's with
Tau's config utility - one for one pad, one for two, and
a keyboard layout.  Make each one then copy it to 
FrontendFiles\Keys, as single.dat, double.dat and keys.dat.
You don't have to make all of the layouts if you'll never
use some, just don't ever choose the option in the loader.

Custom files are supported now.  Files go in FrontendFiles\Custom,
and go in their own folders under the subfolders of Turntables, Frames, 
Announcers, Towels, Notesplashes and BGMs.  Also, all graphic replacements 
need a preview.jpg to display in the frontend - sizes are given below.

Required files:
  Turntables: 0.gcz, preview.jpg
  Towels:     0.gcz, 1.gcz, 2.gcz, preview.jpg
  Frames:     0.gcz, preview.jpg
  Notesplash: 0.gcz, preview.jpg 
  Announcers: sys_op.2dx
  BGMs:       coin_sd.2dx

Preview image sizes:
  Frames:     237x85
  Turntables: 10x14
  Towels:     114x321
  Notesplash: 122x130
  
Better advice on creating the required 0.gcz is included in the
custom files tools pack, released soon.  (you do not have to make
a system.idx file, FrontendFiles is full of pieces that are assembled
to create a working system.idx for normal and custom files as needed)
  
You can't use a custom notesplash and a custom turntable at the same
time.  This is because the GCZ that contains the two files is the same
file.  D'oh.  If you select a custom file, the other option will hide all
of the custom options and turn red.  You can still select a IIDX option when
it's red, but depending on what mischeif the custom GCZ you selected gets
up to, what you see may not be in fact what you get. 

That's about it.  The frontend launches 2007022101\bm2dx_ahn.exe
so make sure your favourite .exe is called that, intel hack, nosync
hack or whatever.  Just choose your options and run it.

It's probably going to go wrong at some point, so restore all your
backup data (if you took it), delete the frontend and be done with it.
No quality assurance on this puppy.

-PoP, PrinceOfPersia@blueyonder.co.uk