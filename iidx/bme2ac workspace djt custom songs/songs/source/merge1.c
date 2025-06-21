//merge1.c by ryuuou
//merges several .1 files into a single one to help with songs that have too many wav references

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHARTCOUNT 7
#define byte unsigned char

const char *ONAMES[] = { "h7.1", "n7.1", "a7.1", "b.1", "h14.1", "n14.1", "a14.1" };
const char *NAMES[] = { "h7", "n7", "a7", "b", "h14", "n14", "a14" };
int offsets[CHARTCOUNT] = {0, 8, 16, 24, 48, 56, 64};
int worder[CHARTCOUNT] = {0, 4, 1, 5, 2, 6, 3};


int main()
{
	int i, len;
	int *plen, *poff;
	byte *onechart[CHARTCOUNT];
	int olen[CHARTCOUNT];
	FILE *in, *out;
	int ccount = 0;

	byte *buf;
	int blen;

	printf("merge1 by ryuuou\n\tmultiplexes several .1 files into a single one\n\n\n");

	in = fopen("base.1", "rb");

	memset(onechart, 0, sizeof(int*)*CHARTCOUNT);
	memset(olen, 0, sizeof(int)*CHARTCOUNT);

	printf("trying to open files...\n");
	printf("base.1...");
	if(!in) printf("not found, authoring new .1 file\n");
	else printf("OK\n");

	if(in) //load base.1 as reference
	{
		fseek(in, 0, SEEK_END);
		blen = (int)ftell(in);
		fseek(in, 0, SEEK_SET);

		buf = malloc(blen);
		fread(buf, blen, 1, in);
		fclose(in);

		printf("loading charts from base.1...");
		//load the charts contained in the base file
		for(i = 0; i < CHARTCOUNT; i++)
		{
			poff = (int*)(buf+offsets[i]);
			plen = (int*)(buf+offsets[i]+4);
			if(!*plen) continue; //no chart

			olen[i] = *plen;

			printf("%s...", NAMES[i]);

			onechart[i] = malloc(olen[i]);

			memcpy(onechart[i], buf+*poff, olen[i]); //copy the chart
		}

		free(buf);
		printf("done\n\n");
	}

	printf("loading and inserting/overwriting charts...\n");

	for(i = 0; i < CHARTCOUNT; i++)
	{
		printf("%s...", ONAMES[i]);
		in = fopen(ONAMES[i], "rb");
		if(!in) 
		{
			printf("not found\n");
			continue;
		}

		ccount++;

		fseek(in, 0, SEEK_END);
		blen = (int)ftell(in);
		fseek(in, 0, SEEK_SET);

		buf = malloc(blen);
		fread(buf, blen, 1, in);
		fclose(in);

		poff = (int*)(buf+offsets[i]);
		plen = (int*)(buf+offsets[i]+4);

		if(!*plen)
		{
			printf("no %s chart\n", NAMES[i]);
			continue; //this file contains no appropriate chart, l2namefiles
		}

		if(olen[i]) //we previously loaded a chart from base.1 and we don't want to leak memory
			free(onechart[i]);

		olen[i] = *plen;

		onechart[i] = malloc(*plen);

		memcpy(onechart[i], buf+*poff, *plen);
		free(buf);
		printf("OK\n");
	}
	
	if(!ccount)
	{
		printf("no charts found, aborting\npress any key to exit\n");
		getchar();
		return 0;
	}

	//create the aggregated .1 file

	len = 96; //.1 headersize

	for(i = 0; i < CHARTCOUNT; i++)
		len += olen[i];

	buf = malloc(len);

	memset(buf, 0, len);

	len = 96; //abuse len as position counter

	printf("\nauthoring .1 file...");
	for(i = 0; i < CHARTCOUNT; i++)
	{
		memcpy(buf+offsets[worder[i]], &len, 4);
		memcpy(buf+offsets[worder[i]]+4, &olen[worder[i]], 4);

		memcpy(buf+len, onechart[worder[i]], olen[worder[i]]);
		len += olen[worder[i]];
	}
	printf("done\n");

	printf("writing merged.1...");
	out = fopen("merged.1", "wb");

	fwrite(buf, len, 1, out);

	fclose(out);
	free(buf);
	printf("done\npress any key to exit\n");

	getchar();
	return 0;
}
