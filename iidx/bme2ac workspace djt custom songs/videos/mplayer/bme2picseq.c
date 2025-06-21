//bme2picseq by ryuuou
//looks up the picture references in .bme files and outputa a sequence of pictures
//those pictures can be used as a base for a 30.00fps movie for IIDX AC

//LOTS of code reuse from bme2ac

#include "common.h"

#define BMS_BPM_CUST 8
#define BMS_BPM 3
#define BMS_STOP 9
#define BMS_MSIZE 2
#define BMS_BGA 4
#define BMS_LAYER 7

#define ONE_TYPE_BPM 4
#define ONE_STOP 0xF
#define ONE_BGA 0x11
#define ONE_LAYER 0x12

typedef struct
{
	int length;
	byte* data;
	float bpmchange[MAXWAVS];
	int bpmchangecount;
	float* msize;
	int mcount;
	float bpm;
	int stopcount;
	float stops[MAXWAVS];

	unsigned char picnames[MAXWAVS+1][64];
	int numpics;
	byte *picdata[MAXWAVS+1];
	int piclen[MAXWAVS+1];
	int layer;
} bmsfile;

typedef struct
{
	unsigned int time;
	byte type;
	byte data1;
	unsigned short int data2;

	void *prev;
	void *next;
} event;


event *onechart;
bmsfile *bms;

void add_event(event ev)
{
	event *temp, *ev_n;

	if(!onechart)
	{
		onechart = malloc(sizeof(event));
		memcpy(onechart, &ev, sizeof(event));
		onechart->prev = onechart->next = NULL;
		return;
	}

	temp = onechart;

	while(temp->next != NULL && temp->time <= ev.time)
	{
		temp = temp->next;
	}

	ev_n = malloc(sizeof(event));
	memcpy(ev_n, &ev, sizeof(event));

	if(temp->time <= ev.time)
	{
		if(temp->next)
			((event*)temp->next)->prev = ev_n;

		ev_n->next = temp->next;
		temp->next = ev_n;

		ev_n->prev = temp;

	}
	else
	{
		if(temp->prev)
			((event*)temp->prev)->next = ev_n;
		else
			onechart = ev_n;

		ev_n->next = temp;
		ev_n->prev = temp->prev;

		temp->prev = ev_n;		
	}
}


void parsebms()
{
	int i,j,k,l;
	char *data;
	event bpmev;
	int measure, channel;
	byte measure_c[4];

	measure_c[3] = '\n';

	printf("parsing bms...");

	data = bms->data;

	for(k = 0; k < bms->length - 5; k++)
	{
		if(data[k] == '#' && data[k+1] == 'B' && data[k+2] == 'P' && data[k+3] == 'M' && data[k+4] == ' ')
		{
			k += 5;
			sscanf((const char*)(data+k), "%f", &(bms->bpm));
			printf("%.2fbpm...", bms->bpm);

			if(bms->bpm-(int)bms->bpm < 0.01 && !(bms->bpm > 655.35))
			{
				bpmev.data1 = 1;
				bpmev.data2 = (unsigned short int)(bms->bpm);
			}
			else
			{
				bpmev.data1 = 100;
				bpmev.data2 = (unsigned short int)bms->bpm*100;
			}
			bpmev.time = 0;
			bpmev.type = ONE_TYPE_BPM;
			add_event(bpmev);
		}
		else if(data[k] == '#' && data[k+1] == 'B' && data[k+2] == 'P' && data[k+3] == 'M' && data[k+4] != ' ')
		{
			j = strtoi(data+k+4) - 1;
			k += 7;
			sscanf((const char*)(data+k), "%f", &(bms->bpmchange[j]));
			bms->bpmchangecount++;
		}
		else if(data[k] == '#' && data[k+1] == 'S' && data[k+2] == 'T' && data[k+3] == 'O' && data[k+4] == 'P')
		{
			j = strtoi(data+k+5) - 1;
			k += 7;
			sscanf((const char*)(data+k), "%f", &(bms->stops[j]));
			bms->stopcount++;
		}
		else if(data[k] == '#' && data[k+1] == 'B' && data[k+2] == 'M' && data[k+3] == 'P')
		{
			l = 0;
			j = strtoi(data+k+4);
			k += 7;
			while((data[k+l] != '\n') && (data[k+l] != '\r') && k+l < bms->length)
				l++;

			memcpy(bms->picnames[j], data+k, l);

			bms->numpics++;
		}
		if(data[k] == '#' && data[k+6] == ':')
		{
			measure_c[0] = data[k+1];
			measure_c[1] = data[k+2];
			measure_c[2] = data[k+3];
			sscanf(measure_c, "%d", &measure);

			if(measure > bms->mcount) bms->mcount = measure;
		}
	}

	bms->msize = malloc(sizeof(float)*(bms->mcount+1));

	for(j = 0; j <= bms->mcount; j++)
		bms->msize[j] = 1.;

	for(j = 0; j < bms->length-6; j++)
	{
		if(data[j] == '#' && data[j+6] == ':')
		{
			sscanf(data+j+4, "%d", &channel);
			if(channel == BMS_MSIZE)
			{
				measure_c[0] = data[j+1];
				measure_c[1] = data[j+2];
				measure_c[2] = data[j+3];
				sscanf(measure_c, "%d", &measure);
				sscanf((const char*)(data+j+7), "%f", &(bms->msize[measure]));
			}
		}
	}
}


int convert_to_1_events()
{
	int i,j,k;
	char measure_c[4];
	int measure,channel;
	float mtime;
	int notecount;
	int denom, nume;

	byte ev_type;
	float ev_time;
	byte ev_data1;
	int bms_ev;
	int count;
	event temp;

	unsigned short int ev_data2;

	char *data;

	measure_c[3] = '\0'; 

	temp.next = temp.prev = NULL;

	printf("building image chain...\n");

	data = bms->data;
	count = 0;

	for(j = 0; j < bms->length-6; j++)
	{
		if(data[j] == '#' && data[j+6] == ':')
		{
			measure_c[0] = data[j+1];
			measure_c[1] = data[j+2];
			measure_c[2] = data[j+3];
			sscanf(measure_c, "%d", &measure);
			sscanf(data+j+4, "%d", &channel);

			ev_data1 = 0;
			ev_data2 = 0;
			ev_time = 0;
			ev_type = 0;
			switch(channel)
			{
				case BMS_BPM:
				case BMS_BPM_CUST:
					ev_type = ONE_TYPE_BPM;
				break;

				case BMS_STOP:
					ev_type = ONE_STOP;
				break;

				case BMS_BGA:
					ev_type = ONE_BGA;
				break;

				case BMS_LAYER:
					ev_type = ONE_LAYER;
				break;

				default:
					continue;
			}

			mtime = 0;
			for(k = 0; k < measure; k++)
				mtime += bms->msize[k];

			denom = 0;
			for(k = j+7; (data[k] != '\n') && (data[k] != '\r') && (data[k] != '#') && (k<=bms->length); k+=2)
				denom++;

			k = j + 7;
			nume = 0;

			for(k = j+7; nume < denom; k+=2)
			{
				ev_time = mtime + ((float)nume/(float)denom)*bms->msize[measure];
				ev_time *= 4. * 60. * 1000./ bms->bpm;
				nume++;

				bms_ev = strtoi(data+k);

				if(!bms_ev)
					continue;

				if(channel == BMS_BPM_CUST)
				{	
					if(bms->bpmchange[bms_ev-1] - (int)bms->bpmchange[bms_ev-1] < 0.01 && !(bms->bpmchange[bms_ev-1] > 655.35))
					{
						ev_data1 = 1;
						ev_data2 = (unsigned short int)(bms->bpmchange[bms_ev-1]);
					}
					else
					{
						ev_data1 = 100;
						ev_data2 = (unsigned short int)bms->bpmchange[bms_ev-1]*100;
					}
				}
				else if(channel == BMS_BPM)
				{
					ev_data1 = 1;
					ev_data2 = (unsigned short)strtoi_h(data+k);
				}
				else if(channel == BMS_STOP)
				{
					ev_data2 = (unsigned short)strtoi(data+k) - 1;
				}
				else if(channel == BMS_BGA)
				{
					count++;
					ev_data2 = (unsigned short)strtoi(data+k);

					if(!bms->piclen[ev_data2])
						ev_data2 = MAXWAVS;
				}
				else if(channel == BMS_LAYER)
				{
					count++;
					bms->layer++;
					ev_data2 = (unsigned short)strtoi(data+k);

					if(!bms->piclen[ev_data2])
						ev_data2 = MAXWAVS;
				}

				temp.time = ev_time;
				temp.type = ev_type;
				temp.data1 = ev_data1;
				temp.data2 = ev_data2;

				add_event(temp);
			}
		}
	}
	return count;
}

void adjust_bpm()
{
	int i;

	float oldbpm = 0;
	float newbpm = 0;
	int stime = 0;
	int tdiff = 0;
	event *temp;
	event help;
	event *tadjust;	

	temp = onechart;

	oldbpm = bms->bpm;

	do
	{
		if(temp->type == ONE_TYPE_BPM)
		{
			newbpm = temp->data2 / temp->data1;
			stime = temp->time;

			if(oldbpm == newbpm)
				continue;
				
			tadjust = temp;

			while(tadjust = tadjust->next)
			{
				tdiff = tadjust->time - stime;
				tdiff = (int)roundf((float)tdiff * oldbpm / newbpm);
				tadjust->time = stime + tdiff;
			}

			oldbpm = newbpm;
		}
	}
	while(temp = temp->next);
}

void add_stop_events()
{
	int i;

	float bpm;

	float tdiff = 0;
	event *temp;
	event *helper;
	event bpm_help;	

	temp = onechart;

	bpm = bms->bpm;

	do
	{
		if(temp->type == ONE_STOP)
		{
			helper = onechart;
			do
			{
				if(helper->type == ONE_TYPE_BPM)
				{
					bpm = (float)helper->data2 / (float)helper->data1;
				}
			} while((helper = helper->next) && (helper->time <= temp->time));

			tdiff = bms->stops[temp->data2] / 192.; 
			tdiff = tdiff * 4. * 60. * 1000. / bpm;

			helper = temp->next;

			do
			{
				if(helper->time > temp->time)
					helper->time += tdiff;
			}
			while(helper = helper->next);
		}
	}
	while(temp = temp->next);
}



int load_pics()
{
	int i, fail=0;
	FILE *f;

	printf("loading picture files...");

	strcpy(bms->picnames[MAXWAVS], "BLACK.bmp");
	bms->numpics++;

	for(i = 0; i <= MAXWAVS; i++)
	{
		if(!(bms->picnames[i][0])) continue;
		f = fopen(bms->picnames[i], "rb");
		if(f == NULL)
		{
			if(!fail)
				printf("failed to load pictures:\n");
			fail++;
			printf("%s ", bms->picnames[i]);
			continue;
		}

		fseek(f, 0, SEEK_END);
   		bms->piclen[i] = (int)ftell(f);
		fseek(f, 0, SEEK_SET);
		
		bms->picdata[i] = malloc(bms->piclen[i]);

		fread(bms->picdata[i],bms->piclen[i], 1, f);
		fclose(f);
		bms->piclen[MAXWAVS] = bms->piclen[i];
	}
	if(fail)
		printf("\nout of %d pictures, %d were not found\n", bms->numpics, fail);
	else
		printf("all pictures loaded successfully\n");

	return fail;
}

void blitbmp(byte* dest, int index, int layer)
{
	int h,w;
	int i, k;
	int rsize;
	byte rs,gs,bs;
	byte *sline, *dline;
	byte *rd,*gd,*bd;
	int offsetw, offseth;

	byte *data;

	if(!bms->piclen[index])
		return;

	data = bms->picdata[index];
	h = *((int*)(data+22));
	w = *((int*)(data+18));
	rsize = w*3;
	if(rsize%4)
		rsize += 4-(rsize%4);

	data += *((int*)(data+10));

	dest += *((int*)(dest+10));

	offsetw = (256 - w) / 2;
	offseth = 256 - h;

	for(i = 0; i < h; i++)
	{
		sline = data + i*rsize;
		dline = dest + ((offseth+i)*256 + offsetw)*3;

		if(!layer)
		{
			memcpy(dline, sline, w*3);
		}
		else
		{
			for(k = 0; k < 3*w; k+=3)
			{
				rs = *(sline+k);
				gs = *(sline+k+1);
				bs = *(sline+k+2);

				rd = dline+k;
				gd = dline+k+1;
				bd = dline+k+2;
				
				if(rs != 0 && gs != 0 && bs != 0)
				{
					*rd = rs;
					*gd = gs;
					*bd = bs;
				}
			}
		}
	}

}

void write_seq()
{
	unsigned char filename[32];
	FILE *out;
	event *temp;
	int i,k,off;
	float time;
	int maxtime;
	int index;
	byte *buf;
	byte *data; 

	temp = onechart;

	buf = malloc(bms->piclen[MAXWAVS]);

	do
	{
		if(temp->type == ONE_BGA)
			maxtime = temp->time + 40;
	}
	while(temp = temp->next);

	time = -6.*1000./30. - 0.005;

	index = MAXWAVS;
	for(i = 0; time < maxtime; i++)
	{
		memcpy(buf, bms->picdata[MAXWAVS], bms->piclen[MAXWAVS]);
 
		temp = onechart;
		do
		{
			if(temp->type == ONE_BGA)
				index = temp->data2;
		}
		while((temp = temp->next) && (temp->time <= time));

		blitbmp(buf, index, 0);

		if(bms->layer)
		{
			temp = onechart;
			do
			{
				if(temp->type == ONE_LAYER)
					index = temp->data2;
			}
			while((temp = temp->next) && (temp->time <= time));

			if(index != MAXWAVS)
				blitbmp(buf, index, 1);
		}

		sprintf(filename, "BMEIMGSEQ_%05d.bmp", i);
		out = fopen(filename, "wb");
		fwrite(buf, bms->piclen[MAXWAVS], 1, out);
		fclose(out);

		time += 1000./30.;
	}

}

int main(int argc, const char* argv[])
{
	FILE *f;

	//printf("bme2picseq v0.1 by ryuuou\n\tProduces a 30fps series of pictures from bme files");


	if(argc < 2)
	{
		printf("USAGE: bme2picseq <file>\n");

		return 0;
	}

	f = fopen(argv[1], "rb");

	if(f == NULL)
	{
		printf("file not found: %s\n", argv[1]);

		//return 0;
	}

	printf("processing %s...", argv[1]);
	bms = malloc(sizeof(bmsfile));
	memset(bms, 0, sizeof(bmsfile));

	fseek(f, 0, SEEK_END);
   	bms->length = (int)ftell(f);
	fseek(f, 0, SEEK_SET);
	bms->data = malloc(bms->length);
	fread(bms->data, bms->length, 1, f);
	fclose(f);

	parsebms();

	load_pics();
	if(!convert_to_1_events())
	{
		printf("no video event, aborting\n");
		return 0;
	}
	adjust_bpm();
	add_stop_events();

	write_seq();

	printf("done.\n");
	//getchar();
	return 0;
}

