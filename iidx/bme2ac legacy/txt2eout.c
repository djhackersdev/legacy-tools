
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

 byte suffix[8];

 int movieoffset;
 byte moviefile[32];

 byte bgname[32];
 byte graphicsfolder[32];

 int layerinfo;
 byte layerdata[9*32];
} songinfo;

int main(int *argc, char* argv[])
{
	eoutheader header;
	songinfo info;
	FILE *in, *out;
	int i;
	int pc, id;
	int myn;
	byte asuffix[9];

	in = fopen(argv[1], "rb");
	if(!in)
	{
		printf("file not found: %s",  argv[1]);
		return 0;
	}

	out = fopen("txt_eout.dec", "wb");

	memset(&header, 0, 16);
	sprintf(header.iidx, "IIDX");
	header.version = 15;

	fwrite(&header, sizeof(eoutheader), 1, out);

	while(!feof(in))
	{
		memset(&info, 0, sizeof(songinfo));
		memset(info.FF, 0xFF, 20);
		memset(info.suffix, 0x30, 8);

		pc = fscanf(in, "%d\t%[a-zA-Z0-9*+<>/\()!? -]\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%[a-z.]\n",\
				&info.index, info.songname, \
				&info.diff_beg, &info.diff_n7, &info.diff_h7, &info.diff_a7, &info.diff_n14, &info.diff_h14, &info.diff_a14,\
				&info.style, &info.sort_others, &info.sort_bemani, &myn, &info.movieoffset, asuffix);

		if(pc != 15)
		{
			printf("parse error in line %d after token %d\n", header.songcount+1, pc);
			break;
		}

		id = info.index;

		sprintf(info.labelname, "L_%d", id);
		sprintf(info.titlename, "T_%d", id);
		sprintf(info.genrename, "G_%d", id);
		sprintf(info.artistname, "A_%d", id);
		sprintf(info.bgname, "B_%d", id);

		sprintf(info.graphicsfolder, "in_%02d", id/100);

		if(myn == 1)
			sprintf(info.moviefile, "%d", id);
		else if(!myn)
			sprintf(info.moviefile, "0000");
		else
			sprintf(info.moviefile, "%d", myn);

		for(i = 0; i < 8; i++)
			if(asuffix[i] == '.')
				asuffix[i] = '0';

		memcpy(info.suffix, asuffix, 8);

		if(info.diff_h7 == 0)
			info.diff_h7 = 1;
		if(info.diff_h14 == 0)
			info.diff_h14 = 1;

		info.sort_bemani2 = info.sort_bemani; //???

		info.volume = 90;

		header.songcount++;

		fwrite(&info, sizeof(songinfo), 1, out);
	}

	rewind(out);
	fwrite(&header, sizeof(eoutheader), 1, out);

	fclose(in);
	fclose(out);

	printf("%d songs in all\n", header.songcount);

	return 0;
}
