//bms.h
//stuff for .bms/.bme processing

//searches *bms for stuff
//WARNING: this is very intolerant concerning capitalization and trailing whitespaces!

#define BMS_MSIZE 2

typedef struct
{
	int length; //length of the data
	byte* data;
	unsigned char wavnames[MAXWAVS][64]; //names of the wavfiles
	int ref[MAXWAVS]; //used for crossreferencing samples across different bms files
	float bpmchange[MAXWAVS]; //bms change array
	int bpmchangecount;
	float msize[999]; //measure sizes
	int mcount; //highest measure count
	float bpm; //initial bpm
	float stops[MAXWAVS]; //stop event array
	int stopcount;
	int lnobj; //lnobj index
	int numwaves; //number of samples
	int numkeys[2]; //keypresscount for player 1 and 2
} bmsfile;

bmsfile bms[CHARTCOUNT]; //one bme for each chart

//attempt to read in the charts; returns the number of loaded charts
int readbmsfiles()
{
	int ret = 0;
	int i;
	FILE *f;

	printf("trying to open charts\n");
	for(i = 0; i < CHARTCOUNT; i++)
	{
		printf("%s...", BMENAMES[i]);
		f = fopen(BMENAMES[i], "rb");
		
		if(f == NULL)
		{
			printf("not found\n");
			continue;
		}

		ret++; //we got a chart, yay

		fseek(f, 0, SEEK_END);
   		bms[i].length = (int)ftell(f);
		fseek(f, 0, SEEK_SET);
		
		bms[i].data = malloc(bms[i].length + 32); //safety bytes
		memset(bms[i].data, 0, bms[i].length + 32);

		fread(bms[i].data, bms[i].length, 1, f);
		fclose(f);
		
		printf("OK\n");
	}
	PNEWLINE;
}

//parses the bms files for data needed to build the timeline, sample index and other stuff
void parsebms()
{
	int i,j,k,l;
	char *data;
	int measure, channel;
	byte measure_c[4];

	printf("parsing bms...");
	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(bms[i].length == 0)
			continue;

		data = bms[i].data;

		for(k = 0; k < bms[i].length; k++)
		{
			l = 0;
			
			//wavefile names
			if(data[k] == '#' && data[k+1] == 'W' && data[k+2] == 'A' && data[k+3] == 'V')
			{
				j = strtoi(data+k+4) - 1;
				k += 7; //skip to the filename (after "#WAVxy ")
				while((data[k+l] != '\n') && (data[k+l] != '\r') && k+l < bms[i].length) //CR or LF
					l++; //count the length of the filename
				memcpy(bms[i].wavnames[j], data+k, l); //copy the wavename
				bms[i].numwaves++;
			}
			//bpm data "#BPM xx"
			else if(data[k] == '#' && data[k+1] == 'B' && data[k+2] == 'P' && data[k+3] == 'M' && data[k+4] == ' ')
			{
				k += 5;
				sscanf((const char*)(data+k), "%f", &bms[i].bpm);
			}
			//bpm data "BPMyy xx"
			else if(data[k] == '#' && data[k+1] == 'B' && data[k+2] == 'P' && data[k+3] == 'M' && data[k+4] != ' ')
			{
				j = strtoi(data+k+4) - 1;
				k += 7;
				sscanf((const char*)(data+k), "%f", &bms[i].bpmchange[j]);
				bms[i].bpmchangecount++;
			}
			//measure data
			else if(data[k] == '#' && data[k+6] == ':')
			{
				measure_c[0] = data[k+1];
				measure_c[1] = data[k+2];
				measure_c[2] = data[k+3];
				sscanf(measure_c, "%d", &measure);
				bms[i].msize[measure] = 1.; //default measure size

				//keep track of the highest measure count for later
				if(measure > bms[i].mcount) bms[i].mcount = measure;
				
				//measure length
				sscanf(data+k+4, "%d", &channel);
				if(channel == BMS_MSIZE)
				{
					sscanf((const char*)(data+k+7), "%f", &bms[i].msize[measure]); //read in the new measure size
				}
			}
			//stop data
			else if(data[k] == '#' && data[k+1] == 'S' && data[k+2] == 'T' && data[k+3] == 'O' && data[k+4] == 'P')
			{
				j = strtoi(data+k+5) - 1;
				k += 7;
				sscanf((const char*)(data+k), "%f", &bms[i].stops[j]);
				bms[i].stopcount++;
			}
			//lnobj data
			else if(data[k] == '#' && data[k+1] == 'L' && data[k+2] == 'N' && data[k+3] == 'O' && data[k+4] == 'B' && data[k+5] == 'J')
			{
				k += 7;
				bms[i].lnobj = strtoi(data+k);
			}
		}	
		printf("%s...", CHARTNAMES[i]);
	}
	PNEWLINE;
}