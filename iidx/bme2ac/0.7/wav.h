//wav.h
//deals with stuff like reading wavfiles and stuff and such stuff

typedef struct
{
	int length;
	char *data;
	char *name;
} WAVE;

WAVE waves[MAXWAVS*CHARTCOUNT]; //wasteful but safe, whatever...
int wavcount; //stores the number of wavefiles to be used in the 2dx file
int wavlen; //stores the sum of length all wavefiles we have loaded

//this fills up "waves" while eliminating duplicates across bme files. while doing so it fills up the bms's structs reference array like this:
//for any bms[i].ref[j], j+1 is the index inside the bme file while ref[j] contains the index which will be used in the .2dx and .1 files
//TODO: What do we do when files are (intentionally?) missing? needs investigation on what 2dx does with 0-byte waves...
void create_wavtable()
{
	int i,j,k;
	int dup = 0;

	printf("building wave index..");
	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(!bms[i].numwaves) continue; //no waves, no fun
		
		printf("%s...", CHARTNAMES[i]);

		for(j = 0; j < MAXWAVS; j++)
		{
			if(bms[i].wavnames[j][0] != 0) //a wave to load
			{
				//now check if we've found a wavefile with the same name before to avoid duplicates
				for(k = 0; k < wavcount; k++)
				{
					if(!strcmp(waves[k].name, bms[i].wavnames[j]))
					{
						dup = 1; //this is a duplicate
						//printf("dup(%d,%d %s == %s)\n", j, k, waves[k].name, bms[i].wavnames[j]);
						break;
					}
				}	

				if(dup)
				{
					bms[i].ref[j] = k+1; //save the reference for later use
					//printf("%s (%d)\n", bms[i].wavnames[j], k);
					dup = 0;
				}
				else //this is a new wavefile
				{
					wavcount++; //iidx starts counting samples at _1_, not 0
					bms[i].ref[j] = wavcount; //save the reference
					waves[wavcount-1].name = bms[i].wavnames[j]; //remember decem^H^H^H^H^H the name of the ga..(ry
					//printf("load(%d,%d %s)\n", j, wavcount, waves[wavcount-1].name);
				}
			}
		}
	}
	printf("added %d wave files to table\n", wavcount);
	return;
}


//actually load the wave files
int load_waves()
{
	int i, fail=0;
	FILE *f;

	printf("loading wave files...");
	for(i = 0; i < wavcount; i++)
	{
		f = fopen(waves[i].name, "rb");
		if(f == NULL)
		{
			if(!fail)
				printf("failed to load waves:\n");
			//printf("wav referenced but not found: %s\n", waves[i].name);
			fail++;
			printf("%s ", waves[i].name);
			continue;
		}

		fseek(f, 0, SEEK_END);
   		waves[i].length = (int)ftell(f);
		fseek(f, 0, SEEK_SET);
		
		waves[i].data = malloc(waves[i].length);

		fread(waves[i].data, waves[i].length, 1, f);
		
		fclose(f);
		//printf("loaded wave %s of length %d\n", waves[i].name, waves[i].length);
	}
	if(fail)
		printf("\nout of %d wave files, %d were not found\n", wavcount, fail);
	else
		printf("all waves loaded successfully\n");
	PNEWLINE;
	return fail;
}
