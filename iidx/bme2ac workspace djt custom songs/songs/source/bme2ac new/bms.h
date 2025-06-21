//bms.h
//stuff for .bms/.bme processing

//searches *bms for stuff
//WARNING: this is very intolerant concerning capitalization and trailing whitespaces!

#define BMS_MSIZE 2

typedef struct
{
	int length;
	byte* data;
	unsigned char wavnames[MAXWAVS][64];
	int ref[MAXWAVS];
	float bpmchange[MAXWAVS];
	int bpmchangecount;
	float* msize;
	int mcount;
	float bpm;
	int stopcount;
	float stops[MAXWAVS];
	int lnobj;
	int numwaves;
	int numkeys[2];
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
		
		bms[i].data = malloc(bms[i].length);

		fread(bms[i].data, bms[i].length, 1, f);
		
		fclose(f);
		printf("OK\n");
	}
	PNEWLINE;
}

//parse the bms files for WAVxy entries and build a list of wav files to load
//THIS BREAKS WITH FILENAMES LONGER THAN 63 BYTES!! But who would do that -.-
void parsebms_wav()
{
	int i,j,k,l;
	char *data;

	printf("parsing bms for wav references...");
	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(bms[i].length == 0)
			continue;

		data = bms[i].data;

		for(k = 0; k < bms[i].length - 6; k++)
		{
			l = 0;
			if(data[k] == '#' && data[k+1] == 'W' && data[k+2] == 'A' && data[k+3] == 'V')
			{
				j = strtoi(data+k+4) - 1;
				k += 7; //skip to the filename (after "#WAVxy ")
				while((data[k+l] != '\n') && (data[k+l] != '\r') && k+l < bms[i].length) //CR or LF
					l++; //count the length of the filename

				//bms[i].wavnames[j] = malloc((l+1)*sizeof(unsigned char));
				memcpy(bms[i].wavnames[j], data+k, l); //copy the wavename

				//bms[i].wavnames[j][l] = '\0'; //terminate the string //everything is zero anyway
				bms[i].numwaves++;
				//break; //we found what we wanted
			}
		}	
		printf("%s: %d...", CHARTNAMES[i], bms[i].numwaves);
	}
	PNEWLINE;
}

//parse the bms files for BPMxy entries, essentially works the same as parsebpm_wav()
void parsebms_bpm()
{
	int i,j,k;
	char *data;

	printf("parsing bms for BPM data...");
	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(bms[i].length == 0)
			continue;

		data = bms[i].data;

		for(k = 0; k < bms[i].length - 5; k++)
		{
			if(data[k] == '#' && data[k+1] == 'B' && data[k+2] == 'P' && data[k+3] == 'M' && data[k+4] == ' ')
			{
				k += 5;
				sscanf((const char*)(data+k), "%f", &bms[i].bpm);
				printf("%s: %.2fbpm...", CHARTNAMES[i], bms[i].bpm);
			}
			else if(data[k] == '#' && data[k+1] == 'B' && data[k+2] == 'P' && data[k+3] == 'M' && data[k+4] != ' ')
			{
				j = strtoi(data+k+4) - 1;
				k += 7;
				sscanf((const char*)(data+k), "%f", &bms[i].bpmchange[j]);

				//printf("found a bpm change (index %d) in chart %s: %f\n", j, CHARTNAMES[i], bms[i].bpmchange[j]);
				bms[i].bpmchangecount++;
			}
		}	
	}
	PNEWLINE;
}

void parsebms_measures()
{
	int i, j;
	int measure, channel;
	byte *data;
	byte measure_c[4];

	measure_c[3] = '\n';

	printf("parsing bms charts for measure data...");
	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(!bms[i].length) continue;

		printf("%s...", CHARTNAMES[i]);

		data = bms[i].data;

		//we now look for '#X' where X is a number
		for(j = 0; j < bms[i].length-6; j++)
		{
			//find a candidate
			if(data[j] == '#' && data[j+6] == ':')
			{
				measure_c[0] = data[j+1];
				measure_c[1] = data[j+2];
				measure_c[2] = data[j+3];
				sscanf(measure_c, "%d", &measure);

				if(measure > bms[i].mcount) bms[i].mcount = measure; //keep track of the highest measure count for later
			}
		}
		
		bms[i].msize = malloc(sizeof(float)*(bms[i].mcount+1));

		for(j = 0; j <= bms[i].mcount; j++)
			bms[i].msize[j] = 1.; //default measure size is 1 (4/4 measure)

		//now that we know the measure count, parse for measure length events
		for(j = 0; j < bms[i].length-6; j++)
		{
			//find a candidate
			if(data[j] == '#' && data[j+6] == ':')
			{
				sscanf(data+j+4, "%d", &channel);

				if(channel == BMS_MSIZE)
				{
					measure_c[0] = data[j+1];
					measure_c[1] = data[j+2];
					measure_c[2] = data[j+3];
					sscanf(measure_c, "%d", &measure);
					sscanf((const char*)(data+j+7), "%f", &bms[i].msize[measure]); //read in the new measure size
					//printf("Measure size change in %s (m:%d, v:%f)\n", CHARTNAMES[i], measure, bms[i].msize[measure]);
				}
			}
		}
		//TODO: Do we need measure size events for the .1 file?
	}
	PNEWLINE;
}

void parsebms_stops()
{
	int i,j,k;
	char *data;

	printf("parsing bms for stops...");
	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(bms[i].length == 0)
			continue;

		data = bms[i].data;

		for(k = 0; k < bms[i].length - 6; k++)
		{
			if(data[k] == '#' && data[k+1] == 'S' && data[k+2] == 'T' && data[k+3] == 'O' && data[k+4] == 'P')
			{
				j = strtoi(data+k+5) - 1;
				k += 7;
				sscanf((const char*)(data+k), "%f", &bms[i].stops[j]);

				//printf("found a stop (index %d) in chart %s: %f\n", j, CHARTNAMES[i], bms[i].stops[j]);
				bms[i].stopcount++;
			}

			if(data[k] == '#' && data[k+1] == 'L' && data[k+2] == 'N' && data[k+3] == 'O' && data[k+4] == 'B' && data[k+5] == 'J')
			{
				k += 7;
				bms[i].lnobj = strtoi(data+k);
				//sscanf((const char*)(data+k), "%d", &bms[i].lnobj);

				//printf("found a lnobj in chart %s: %d\n", CHARTNAMES[i], bms[i].lnobj);
			}
		}
	}
	PNEWLINE;
}

