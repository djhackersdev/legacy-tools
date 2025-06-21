//parser.c

#include <stdio.h>
#include <stdlib.h>

const char *BMENAMES[] = { "Hyper7", "Normal7", "Another7", "Beginner", "Hyper14", "Normal14", "Another14" };
int offsets[7] = {0, 8, 16, 24, 48, 56, 64};

#define byte unsigned char

int main(int argc, const char **argv)
{
	FILE *f;
	int length;
	byte *buffer, event, *data1;
	int i,k,pos;
	byte *chart;
	int chartl,offset;
	int maxwav = 0;
	int minwav = 9999;
	int *time;
	int evcount;
	int mcount;
	int v=1;
	unsigned short int *data2;

	printf("parse1 by ryuuou\n\tparses .1 files and prints out event information\n");

	if(argc < 2)
	{
		printf("USAGE: parse1 <file> <verbosity>\n");
		printf("\tverbosity:\n\t\tv (default): print standard info and summary only\n\t\tvv: same as v, also print bpm info and measure related events\
\n\t\tvvv: print everything except for keysound preload and autoplay events\n\t\tvvvv: print everything\n\n");
		getchar();
		return 0;
	}
	f = fopen(argv[1], "rb");
	if(!f) return 1;

	if(argc > 2)
	{
		if(!strcmp(argv[2], "vvvv"))
			v = 4;
		else if(!strcmp(argv[2], "vvv"))
			v = 3;
		else if(!strcmp(argv[2], "vv"))
			v = 2;
	}

	fseek(f, 0, SEEK_END);
	length = (int)ftell(f);
	fseek(f, 0, SEEK_SET);

	buffer = malloc(length);
	fread(buffer, length, 1, f);

	printf("output format: <eventtime>(<fileposition in hex>): <event type> <event data info>\n\n");
	for(k = 0; k < 7; k++)
	{
	offset = *((int*)(buffer+offsets[k]));
	chartl = *((int*)(buffer+offsets[k]+4));
	if(!chartl) continue;
	
	printf("\nchart %s\n", BMENAMES[k]);

	maxwav=0;
	minwav=9999;
	evcount = 0;
	mcount = 0;
	for(i = offset + 4; i <= offset+chartl -4; i+=8)
	{
		evcount++;
		data1 = buffer+1+i;
		data2 = (unsigned short int *)(buffer+i+2);
		time = (int*)(buffer+i-4);
		//printf("data2: %d")
		if(*time == 0x7FFFFFFF)
		{
			printf("%x(%x): END SEQUENCE EVENT\n", *time, i);
			continue;
		}

		switch(buffer[i])
		{
			case 2: //keysound preload
			case 3:
				if(*data2 < minwav) minwav = *data2;
				if(*data2 > maxwav) maxwav = *data2;
			case 0: //keypress, preloads
			case 1:
				switch(*data1)
				{
					case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:

						if(buffer[i] == 3 || buffer[i] == 2) {if(v>3) printf("%d(%x): LOAD KEY%d P%d: SAMPLE %d\n", *time, i, *data1+1, buffer[i]-1, *data2);}
						else {if(v>2)printf("%d(%x): KEY EVENT: KEY%d P%d\n", *time, i, *data1+1, buffer[i]+1);}
						continue;
					break;
				}
			break;
			case 4: //bgm change
				if(*data1 == 1 || *data1 == 100)
				{
					if(v>1)printf("%d(%x): BPM CHANGE EVENT: %d %d\n", *time, i, *data1, *data2);
					continue;
				}
			break;
			case 5: //measure size
				{
					if(v>1)printf("%d(%x): MEASURE SIZE EVENT\n", *time, i);
					if(*data1 == 4) continue;
				}
			break;
			break;
			case 6: //end song
				if(*data1 == 0 || *data1 == 1)
				{
					printf("%d(%x): ENDSONG MARKER for player %d\n", *time, i, *data1+1);
					continue;
				}
			break;
			case 7: //autoplay
				if(*data1 == 0);
				{
					if(*data2 < minwav) minwav = *data2;
					if(*data2 > maxwav) maxwav = *data2;
					if(v>3) printf("%d(%x): AUTOPLAY SAMPLE %d\n", *time, i, *data2);
					continue;
				}
			case 8: //jugdement
				switch(*data1)
				{
					case 0: case 1: case 2: case 3: case 4: case 5:
						continue;
					break;
				}
			break;
			case 0xC: //measure line
				if(*data1 == 0 || *data1 == 1)
				{
					if(*data1 == 0) mcount++;
					if(v>1) printf("%d(%x): MEASURE LINE EVENT P%d (%d)\n", *time, i, *data1+1, mcount);
					continue;
				}
			break;
			case 0x10:
				if(*data1 == 0 || *data1 == 1)
				{
					printf("%d(%x): NOTECOUNT EVENT: P%d %d\n", *time, i, *data1+1, *data2);
					continue;
				}
				break;
			default:
			break;
		}

		printf("UNKNOWN EVENT AT %d(%x): ID:%d DATA1:%d DATA2:%d\n", *time, i, buffer[i], *data1, *data2);

	}
	printf("sample preload indices: highest: %d, lowest: %d\n", maxwav, minwav);
	printf("%d events in all\n", evcount);
	}
	return 0;
}
