#define byte unsigned char

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "crypt.h"

typedef struct
{
 byte iidx[4];
 int version;
 short int songcount;
 short int indexcount;
 int zero;
} eoutheader;

typedef struct
{
 byte songname[64];
 byte labelname[32]; //filename
 byte titlename[64]; //filename
 byte genrename[32]; //filename
 byte artistname[32]; //filename
 
 short int style;
 short int sort_others;
 short int sort_bemani;
 short int sort_bemani2;

 byte diff_n7;
 byte diff_h7;
 byte diff_a7;
 byte diff_n14;
 byte diff_h14;
 byte diff_a14;
 byte diff_beg;
 byte diff_ex;

 byte zeros[128];
 byte FF[20];

 int index;
 int volume;

 char suffix[8];

 int movieoffset; //used as temp var here
 byte moviefile[32];

 byte bgname[32];
 byte graphicsfolder[32];

 int layerinfo;
 byte layerdata[9*32];
} songinfo;

typedef struct
{
	int offh7;
	int lenh7;
	int offn7;
	int lenn7;
	int offa7;
	int lena7;
	int offb;
	int lenb;
	int offex1;
	int lenex1;
	int offex2;
	int lenex2;
	int offh14;
	int lenh14;
	int offn14;
	int lenn14;
	int offa14;
	int lena14;
} oneheader;

eoutheader ehead;
songinfo cursong;
unsigned short int judgements[6] = {0xF0, 0xFA, 0xFF, 0x3, 0x8, 0x12};

#define PRINTW(a,b) if(warn) {printf("__: %d (%s) - ", cursong.index, cursong.songname);printf(a,b);}
#define PRINTA(a,b) if(warn) {printf("**: %d (%s) - ", cursong.index, cursong.songname);printf(a,b);}
#define PRINTE(a,b) if(err) {printf("!!: %d (%s) - ", cursong.index, cursong.songname);printf(a,b);}

int main(int argc, char* argv[])
{
	FILE *eout, *one, *tdx, *movie;
	char file[60];
	char onepart[2], prev, *diff;
	oneheader onehead;
	int warn, err;
	int k, i, l, songcount;
	int tdxlen;
	byte *enc, *dec;
	short int fmt;
	int pos, wavcount, wavlen;

	if(argc < 2)
	{
		printf("checkeout <decrypted eout> <level>\n\tlevel: v = errors only\n\tvv = warnings only\n\t(default) = errors and warnings");
		return 0;
	}

	warn = err = 1;

	if(argc > 2)
	{
		if(!strcmp(argv[2], "vv"))
			err = 0;
		else if(!strcmp(argv[2], "v"))
			warn = 0;
	}

	//warn=err=0;
	eout = fopen(argv[1], "rb");

	if(!eout)
	{
		printf("could not find %s, aborting\n", argv[1]);
		return 0;
	}

	fread(&ehead, sizeof(eoutheader), 1, eout);

	if(strncmp(ehead.iidx, "IIDX", 4))
	{
		printf("%s is not a decrypted eout.bin file, aborting\n", argv[1]);
		return -1;
	}

	fseek(eout, sizeof(eoutheader) + ehead.indexcount*2, SEEK_SET);

	songcount = 0;

	//while(!feof(eout))
	for(k = 0; k < ehead.songcount; k++)
	{
		fread(&cursong, sizeof(songinfo), 1, eout);
		songcount++;
		//printf("ID: %d\tNAME: %s || %d\n", cursong.index, cursong.songname, cursong.style);

		//check for movie file
		sprintf(file, "movie/%s.4", cursong.moviefile);
		if(!(movie = fopen(file, "rb")))
		{PRINTW("moviefile not found: %s\n", file)}
		else
			fclose(movie);

		//check mendatory difficulties
		if(!cursong.diff_h7)
		{PRINTE("song has no hyper7 chart\n", NULL)}

		if(!cursong.diff_h14)
		{PRINTE("song has no hyper14 chart\n", NULL)}

		//check .1 files
		sprintf(file, "sd_data/%04d/%04d.1", cursong.index, cursong.index);
		printf("checking %s (%d)...\n", file, k);
		if(!(one = fopen(file, "rb")))
		  {PRINTE(".1 file not found: %s\n", file)}
		else
		{
			//check if difficulties are availible
			fread(&onehead, sizeof(onehead), 1, one);
			fclose(one);

			if(cursong.diff_n7 && !onehead.offn7)
				PRINTE("referenced n7 (%d) chart not found in .1 file\n", cursong.diff_n7)
			if(cursong.diff_h7 && !onehead.offh7)
				PRINTE("referenced h7 (%d) chart not found in .1 file\n", cursong.diff_h7)
			if(cursong.diff_a7 && !onehead.offa7)
				PRINTE("referenced a7 (%d) chart not found in .1 file\n", cursong.diff_a7)
			if(cursong.diff_n14 && !onehead.offn14)
				PRINTE("referenced n14 (%d) chart not found in .1 file\n", cursong.diff_n14)
			if(cursong.diff_h14 && !onehead.offh14)
				PRINTE("referenced h14 (%d) chart not found in .1 file\n", cursong.diff_h14)
			if(cursong.diff_a14 && !onehead.offa14)
				PRINTE("referenced a14 (%d) chart not found in .1 file\n", cursong.diff_a14)
			if(cursong.diff_beg && !onehead.offb)
				PRINTE("referenced beg (%d) chart not found in .1 file\n", cursong.diff_beg)
			if(cursong.diff_ex && !onehead.offex1)
				PRINTE("referenced ex (%d) chart not found in .1 file\n", cursong.diff_ex)

			if(!cursong.diff_n7 && onehead.offn7)
				PRINTW("unreferenced n7 chart in .1 file\n", NULL)
			if(!cursong.diff_h7 && onehead.offh7)
				PRINTW("unreferenced h7 chart in .1 file\n", NULL)
			if(!cursong.diff_a7 && onehead.offa7)
				PRINTW("unreferenced a7 chart in .1 file\n", NULL)
			if(!cursong.diff_n14 && onehead.offn14)
				PRINTW("unreferenced n14 chart in .1 file\n", NULL)
			if(!cursong.diff_h14 && onehead.offh14)
				PRINTW("unreferenced h14 chart in .1 file\n", NULL)
			if(!cursong.diff_a14 && onehead.offa14)
				PRINTW("unreferenced a14 chart in .1 file\n", NULL)
			if(!cursong.diff_beg && onehead.offb)
				PRINTW("unreferenced beg chart in .1 file\n", NULL)
			if(!cursong.diff_ex && onehead.offex1)
				PRINTA("unreferenced ex chart in .1 file\n", NULL)

			if(cursong.diff_ex)
				PRINTA("ex chart (%d) in eout\n", cursong.diff_ex)
			if(onehead.offex1)
				PRINTA("ex1 chart in .1 file\n", NULL)
			if(onehead.offex2)
				PRINTA("ex2 chart in .1 file\n", NULL)

			//todo: .1 file sanity check?
		}


		//check .2dx files
		onepart[0] = prev = 0xFF;
		onepart[1] = '\0';
		diff = &cursong.diff_n7;

		for(i = 0; i <= 7; i++)
		{
			if(!diff[i])
				continue;
			if(prev == cursong.suffix[i])
				continue;
			else
				onepart[0] = prev = cursong.suffix[i];

			if(onepart[0] == '0')
				onepart[0] = '\0';

			sprintf(file, "sd_data/%04d/%04d%s.2dx", cursong.index, cursong.index, onepart);
			//printf("checking %s (%d)...\n", file, k);
			if(!(tdx = fopen(file, "rb")))
			{PRINTE(".2dx file not found: %s\n", file)}
			else
			{
				//printf("decoding: %s...", file);
				fseek(tdx, 0, SEEK_END);
   				tdxlen = (int)ftell(tdx);
				if(tdxlen > 25000000)
				{PRINTE("%s larger than 25,000,000 bytes\n", file)}
				rewind(tdx);
				enc = malloc(tdxlen);
				fread(enc, 1, tdxlen, tdx);
				fclose(tdx);
				decrypt_2dx(enc, &dec);
				//printf("done\n");
				memcpy(&wavcount, dec + 20, 4);
				pos = 72;
				
				for(l=0; l < wavcount-1; l++)
				{
				  memcpy(&fmt, dec + *((int*)(dec+pos)) + 24 + 20, 2);
				  memcpy(&wavlen, dec + *((int*)(dec+pos)) + 8, 4);
				  pos += 4;
				  //printf("(%d %d %d)\n", wavcount, pos, *((int*)(dec+pos)));
				  /*if(wavlen == 0)
				  {
				    {PRINTE("%s has 0-length wav files\n", file)}
				    break;
				  }
				  if(fmt != 2)
				  {
				    {PRINTE("%s has unconverted wav files\n", file)}
				    break;
				  }*/
				}
				free(enc);
				free(dec);
			}
		}
	}

	printf("songcount: %d", songcount);

	getchar();

	return 0;
}
