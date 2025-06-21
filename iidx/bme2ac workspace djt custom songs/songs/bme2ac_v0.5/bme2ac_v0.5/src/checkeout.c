#define byte unsigned char

#include "stdio.h"
#include "string.h"

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
 byte diff_zero;

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
	int dummy1;
	int dummy2;
	int dummy3;
	int dummy4;
	int offh14;
	int lenh14;
	int offn14;
	int lenn14;
	int offa14;
	int lena14;
} oneheader;

eoutheader ehead;
songinfo cursong;

#define PRINTW if(warn) {printf("__: %d (%s) - ", cursong.index, cursong.songname)
#define PRINTE if(err) {printf("!!: %d (%s) - ", cursong.index, cursong.songname)

int main(int argc, char* argv[])
{
	FILE *eout, *sys, *one, *tdx, *movie;
	char file[34];
	char onepart[2], prev, *diff;
	oneheader onehead;
	int *iptr;
	int warn, err;
	int i;
	int tdxlen;

	if(argc < 2)
	{
		printf("checkeout <decoded eout> <level>\n\tlevel: v = errors only\n\tvv = warnings only\n\t(default) = errors and warnings");
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

	eout = fopen(argv[1], "rb");

	if(!eout)
	{
		printf("could not find %s, aborting\n", argv[2]);
		return 0;
	}

	fread(&ehead, sizeof(eoutheader), 1, eout);

	fseek(eout, sizeof(eoutheader) + ehead.indexcount*2, SEEK_SET);

	while(!feof(eout))
	{
		fread(&cursong, sizeof(songinfo), 1, eout);
		//printf("ID: %d\tNAME: %s\n", cursong.index, cursong.songname);

		//check for movie file
		sprintf(file, "movie/%s.4", cursong.moviefile);
		if(!(movie = fopen(file, "rb")))
			{
				PRINTW;
				printf("moviefile not found: %s\n", file); }
			}
		else
			fclose(movie);

		//check difficulties
		if(!cursong.diff_h7)
		{
			PRINTE;
			printf("song has no hyper7 chart\n"); }
		}
		if(!cursong.diff_h14)
		{
			PRINTE;
			printf("song has no hyper14 chart\n"); }
		}

		//check .1 files
		sprintf(file, "sd_data/%04d/%04d.1", cursong.index, cursong.index);
		if(!(one = fopen(file, "rb")))
			{
				PRINTE;
				printf(".1 file not found: %s\n", file); }
			}
		else
		{
			//check if difficulties are availible
			fread(&onehead, sizeof(onehead), 1, one);
			fclose(one);

			if(cursong.diff_n7 && !onehead.offn7)
				PRINTE; printf("referenced n7 (%d) chart not found in .1 file\n", cursong.diff_n7); }
			if(cursong.diff_h7 && !onehead.offh7)
				PRINTE; printf("referenced h7 (%d) chart not found in .1 file\n", cursong.diff_h7); }
			if(cursong.diff_a7 && !onehead.offa7)
				PRINTE; printf("referenced a7 (%d) chart not found in .1 file\n", cursong.diff_a7); }
			if(cursong.diff_n14 && !onehead.offn14)
				PRINTE; printf("referenced n14 (%d) chart not found in .1 file\n", cursong.diff_n14); }
			if(cursong.diff_h14 && !onehead.offh14)
				PRINTE; printf("referenced h14 (%d) chart not found in .1 file\n", cursong.diff_h14); }
			if(cursong.diff_a14 && !onehead.offa14)
				PRINTE; printf("referenced a14 (%d) chart not found in .1 file\n", cursong.diff_a14); }
			if(cursong.diff_beg && !onehead.offb)
				PRINTE; printf("referenced beg (%d) chart not found in .1 file\n", cursong.diff_beg); }

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
			if(!(tdx = fopen(file, "rb")))
				{
					PRINTE;
					printf(".2dx file not found: %s\n", file); }
				}
			else
			{
				fseek(tdx, 0, SEEK_END);
   				tdxlen = (int)ftell(tdx);
				if(tdxlen > 25000000)
					PRINTE; printf(".2dx larger than 25,000,000 bytes\n"); }
				fclose(tdx);
			}
		}
	}

	return 0;
}
