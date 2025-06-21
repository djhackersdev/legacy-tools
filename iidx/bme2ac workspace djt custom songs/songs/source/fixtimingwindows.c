//timebase_single.c by ryuuou
//changes the base fps value of .1 files like timebase.exe, but works on a bunch of single .1 files instead of folders
//this is mainly to assist with custom songs so that you don't have to timebase your whole folder again for single songs and deal with file versions

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define byte unsigned char

#define CHARTCOUNT 7
#define EVENTSIZE 8
#define FPS_STANDARD_GOLD 60.05
#define FPS_STANDARD_DD 59.95

int offsets[CHARTCOUNT] = {0, 8, 16, 24, 48, 56, 64};
const char *CHARTNAMES[] = { "h7", "n7", "a7", "b", "h14", "n14", "a14" };
unsigned short int judgements[6] = {0xF0, 0xFA, 0xFF, 0x3, 0x8, 0x12};

typedef struct
{
	unsigned int time;
	byte type;
	byte data1;
	unsigned short int data2;
} event;

int main(int argc, const char **argv)
{
	int i,j,k;
	FILE *in, *out;
	byte *buf;
	int *p;
	int flength;
	int pos, len;
	short int np1, np2;

	event *ev;

	printf("fixtimingwindows by ryuuou\n\tfixes timing windows to be consistent\n\n");
	if(argc < 2)
	{
		printf("usage: fixtimingwindows <file1> <file2> ...\n\talternatively drop a bunch of .1 files on the executable\n");
		getchar();
		return 0;
	}

	for(i = 2; i <= argc; i++)
	{
		printf("processing %s...", argv[i-1]);
		in = fopen(argv[i-1], "rb");
		if(!in)
		{
			printf("file not found\n");
			continue;
		}

		fseek(in, 0, SEEK_END);
   		flength = (int)ftell(in);
		fseek(in, 0, SEEK_SET);
		
		buf = malloc(flength);

		fread(buf, flength, 1, in);
		fclose(in);	

		for(j = 0; j < CHARTCOUNT; j++)
		{
			p = (int*)(buf+offsets[j]);
			if(!*p) continue; //no chart here

			//printf("%s...\n", CHARTNAMES[j]);

			pos = p[0];
			len = p[1];

			np1 = 0;
			np2 = 0;
			
			for(k = pos; k < pos+len - EVENTSIZE; k += EVENTSIZE)
			{
				ev = (event*)(buf+k);
				
				if(ev->type == 8) //adjust timing
				{
					//printf("%d (%d->%d)...", ev->data1, ev->data2, judgements[ev->data1]);
					ev->data2 = judgements[ev->data1];
				}
				if(ev->type == 0)//player 1 keypress
				{
				  np1++;
				}
				else if(ev->type == 1) //player 2 keypress
				{
				  np2++;
				}
			}
			
			for(k = pos; k < pos+len; k += EVENTSIZE)
			{
				ev = (event*)(buf+k);
				
				if(ev->type == 0x10)//notecount
				{
				  if(ev->data1 == 0) //player1
				  {
				    printf("%d -> %d | ", ev->data2, np1);
				    ev->data2 = np1;
				  }
				  else if(ev->data1 == 1) //player2
				  {
				    printf("%d -> %d\n", ev->data2, np2);
				    ev->data2 = np2;
				  }
				}
			}
			//printf("%s:  p1: %d, p2: %d\n", CHARTNAMES[j], np1, np2);
		}

		out = fopen(argv[i-1], "wb");
		fwrite(buf, flength, 1, out);
		fclose(out);

		free(buf);

		printf("done\n");
	}
}
