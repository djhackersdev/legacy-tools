//create2dx_dumb.c by kinryuu based on create2dx.c by Kitaru, but aside from some variables not much is left ;D
//Packs BME-style sequenced .wav (01.wav to ZZ.wav, but should work with BMS style 01 - FF as well) in the current folder into an unencrypted output.2dx
//Takes no arguments and always outputs to the same file; only works with uppercase on unix but should work with lowercase on windows (if at all);
//We also assume that the ascending order or the wavefiles equals their order in the corresponding bme/bms file
//...hence "_dumb"


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//wav files from 01-0A-0Z-10-ZZ (you get it) = 36*36 - 1
#define MAXWAVS (36*36-1)

typedef unsigned char byte;

const byte aftertoc[24] = {
   	'2', 'D', 'X', '9', //const string
	0x18, 0x00, 0x00, 0x00, //headersize (== 24)
	0x00, 0x00, 0x00, 0x00, //size of wave file (written at runtime)
	0x30, 0x32, 0xFF, 0xFF, //panning/initial volume?
	0x40, 0x00, 0x08, 0x00, //??? (64, 0, 8, 0)
	0x00, 0x00, 0x00, 0x00  //??? (0, 0, 0, 0)
};

typedef struct{
	int length;
	byte *data;
} wavefile;

int main()
{
	//coding while listening to kachoufuusetsu long, hell yeah!

	wavefile waves[MAXWAVS]; //use this to hold the wavefiles; MAYBE I SHOULD NAME THIS SMALL_WAVES ZOMG GET IT?

	FILE *wavfile;
	FILE *out;

	char digit1, digit2; //use this to hold the XY of [XY].wav
	int wav = 0; //wavefile count
	int firstwav = 0; //position of the first wave in the .2dx file (needed in .2dx header)
	int pos = 0, i = 0;

	char wavepath[7]; //helperstring; ugly but works

	memset(waves, 0, MAXWAVS*sizeof(wavefile));

	i = 1; //BME starts counting at 01.wav, how stupid -_-

	while(i <= MAXWAVS)
	{
		digit1 = i / 36; //works because this floors
		if(digit1 > 9)
			digit1 += 7; //there's a gap of 8 characters between '9' and 'A'
		digit1 += 48; //fancy charactermap magic

		digit2 = i % 36; //mod operator = <3
		if(digit2 > 9)
			digit2 += 7; 
		digit2 += 48;	
		sprintf(wavepath,"%c%c.wav",digit1,digit2);

		wavfile = fopen(wavepath, "rb");
		if (wavfile != NULL)
		{
			//open the wave file and save its length and contents to the array
			printf("reading %c%c.wav...",digit1,digit2);
			fseek(wavfile, 0, SEEK_END);
   		 	waves[i-1].length = (int)ftell(wavfile);
			fseek(wavfile, 0, SEEK_SET);
			printf("length %d...", waves[i-1].length);

			waves[i-1].data = malloc(waves[i-1].length);

			fread(waves[i-1].data, waves[i-1].length, 1, wavfile);

			fclose(wavfile);
			wav++;
			printf("OK\n");
		}
		i++;
	}

	if(wav == 0) //learn2placefilesindirectories
	{
		printf("no wave files found, go die in a fire\n");
		return 0;
	}

	printf("\ntotal samplecount: %d\n\n",wav);

	out = fopen("output.2dx", "wb"); //open the 2dx file

	fseek(out, 0x10, SEEK_SET); //ignore the first 16 bytes (TODO: zero them to be sure?)

	firstwav = 72 + wav*4; //first wave position = 2dx header + size of TOC (= wavecount * sizeof(int))
	fwrite(&firstwav, 4, 1, out); //write the position of the first wave file
	fwrite(&wav, 4, 1, out); //write the total numbers of waves

	fseek(out, 0x48, SEEK_SET);

	printf("writing TOC...");

	pos = 0x48 + ((wav) * 4); //start of the wavefiles
	//write the table of contents (absolute position of the waves in the .2dx file)
	for(i = 0; i < MAXWAVS; i++)
	{
		if(waves[i].length > 0) //we only want waves with content
		{
			printf("%d...",pos);
			fwrite(&pos, 4, 1, out);
			pos += waves[i].length + 0x18; //don't forget about the 24 byte header
		}
	}
	printf("OK!\n\n");

	//write the actual waves
	printf("writing samples...");
	for(i = 0; i < MAXWAVS; i++)
	{
		if(waves[i].length > 0)
		{
			printf("%d ", i);
			fwrite(aftertoc, 8, 1, out); //"2DX9" + headersize (== 24)
			fwrite(&waves[i].length, 4, 1, out); //size of the wave
			printf("(%d)...", waves[i].length);
			fwrite(aftertoc+12, 12, 1, out); //rest of the header
			fwrite(waves[i].data, waves[i].length, 1, out);
		}
	}
	printf("OK!\n");
	fclose(out);	
	return 0;
}
