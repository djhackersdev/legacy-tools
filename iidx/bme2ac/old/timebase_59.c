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

int main(int argc, const char **argv)
{
	int i,j,k;
	float basefps, targetfps;
	FILE *in, *out;
	byte *buf;
	int *p;
	int flength;
	int pos, len;

	printf("timebase_single by ryuuou\n\tchanges the base fps of .1 files like timebase.exe,\n\tbut works on a batch of files instead of folders\n\n");
	if(argc < 2)
	{
		printf("usage: timebase_single <file1> <file2> ...\n\talternatively drop a bunch of .1 files on the executable\n");
		getchar();
		return 0;
	}

	basefps = targetfps = 0;

	/*printf("please input the current fps (or 0 for 60.05 or 1 for 59.95): ");
	scanf("%f", &basefps);
	if(basefps == 0)
		basefps = FPS_STANDARD_GOLD;
	if(basefps == 1)
		basefps = FPS_STANDARD_DD;
	printf("please input the target fps: (or 0 for 60.05 or 1 for 59.95)");
	scanf("%f", &targetfps);
	if(targetfps == 0)
		targetfps = FPS_STANDARD_GOLD;
	if(targetfps == 1)
		targetfps = FPS_STANDARD_DD;


	printf("converting from %.3f fps to %.3f fps\npress enter to continue or ctrl-c to cancel\nMAKE SURE YOU HAVE BACKUPS OF YOUR FILES!!\n", basefps, targetfps);
	getchar();getchar();*/

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

			printf("%s...", CHARTNAMES[j]);

			pos = p[0];
			len = p[1];

			for(k = pos; k < pos+len; k += EVENTSIZE)
			{
				p = (int*)(buf+k);
				if(*p == 0x7FFFFFFF || *p == 0x7F000000) //end sequence event, we skip these
					continue;
				*p = (int)roundf((float)*p *1000. / FPS_STANDARD_DD);
			}
		}

		out = fopen(argv[i-1], "wb");
		fwrite(buf, flength, 1, out);
		fclose(out);

		free(buf);

		printf("done\n");
	}
}
