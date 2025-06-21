//bme2ac by ryuuou
//takes a bunch of bms/bme files and produces .2dx and .1 files (hopefully) usable with beatmania iidx ac
//!!!WARNING!!! THIS WILL PROBABLY FAIL HORRIBLY ON 64BIT PLATFORMS!!! Then again it might just work

//still unsupported:
//	- songs with missing wav files?

//Changelog:
// v0.6
//	- greater accuracy in case of many bpm changes
//	- proper longnote handling support
// v0.5
//	- partial support for freezenotes
// v0.4
//	- speedups
//	- implemented #STOP events (_NOT_ #STP events!)
//	- fixed a potential problem with fractional bpm
//	- fixed a bug with file names that have '#' in them
//	- the measure line of the last measure wasn't written out correctly
//	- nixed decrypted.2dx; encrypted.2dx -> output.2dx
//	- songs now end 3 seconds after the end of the last measure
//	- added several tools:
//	  create2dx_new, crack2dx_new, timebase_single, encrypt2dx, merge1, parse1; with sources
// v0.3
//	- killed debug output
//	- standard events are now written after bpm corrections to avoid screwing with the end-song event
//	- keysound preload events are now dynamic and always placed between keys; this should fix wrong keysounds on fast repeats once and for all
// v0.2
//	- implemented bpm changes
//	- implemented changing measure sizes, this should fix a lot of offsync songs
// v0.1
//	- initial release


#include "common.h"

#include "bms.h"
#include "wav.h"
#include "2dx.h"
#include "crypt.h"
#include "1.h"
#include <math.h>

//initialize stuff, set things to zero etc.
void init()
{
	memset(bms, 0, sizeof(bmsfile)*CHARTCOUNT);
	memset(waves, 0, sizeof(WAVE)*MAXWAVS*CHARTCOUNT);
	
	memset(onecharts, 0, sizeof(event*)*CHARTCOUNT);

	wavcount = 0;
	wavlen = 0;
	onesize = 0;
	onefile = NULL;

	twodxsize = 0;

	style = 0; //standard = troopers
	return;
}

int main()
{
	init();

	printf("bme2ac v0.6 by ryuuou\n \
	Converts BME/BMS style files and WAVs into .1 and .2dx files\n\
	Suitable for use with beatmania IIDX TROOPERS AC and up _only_\n\n\n");

	if(!readbmsfiles())
	{
		printf("found no charts, aborting\n");
		getchar();
		return 1;
	}

	parsebms_wav();

	parsebms_bpm();
	parsebms_measures();
	parsebms_stops();
	create_wavtable();
	if(load_waves())
	{
		printf("!!!Several wave files failed to load, IIDX might crash!!!\npress enter to continue\n");
		getchar();
		//return 1;
	}

	convert_to_1_events();

	handle_longnotes();

	stretch_times(100.); //for greater accuracy in case of many bpm changes
	adjust_bpm();
	stretch_times(1./100.);
	//note: we still have inaccuracy of 1ms while converting from bms, but there's nothing we can do about that

	add_keysounds();
	cleanup_1();

	add_standard_events();

	add_stop_events(); //we need at least one bpm event for calculations

	//stretch_times(FPS_DD/1000.); //for DistorteD to Happy Sky
	//stretch_times(FPS_GOLD/1000.); //for GOLD

	create_1();

	create_2dx();
	encrypt_2dx(tdx, twodxsize, &tdx_enc);

	writeout_1();
	writeout_2dx();

	if(twodxsize > 25000000)
		printf("!!!.2dx file is larger than 25,000,000 bytes, IIDX will crash trying to load it!!!\n");

	printf("\nDone. Press enter to exit.\n");
	//getchar();
	return 0;
}

