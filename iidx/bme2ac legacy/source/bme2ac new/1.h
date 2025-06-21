//1.h
//deals with parsing the bms files for events and converting them into a chain of .1 file events and other .1 related stuff
//largely based on DXAC1.pas (QQQ)

//2dx ac framerate
#define FPS_GOLD 60.05 //According to Tau GOLD uses this framerate instead of the standard 60.04 for internal computations
#define FPS_DD 59.95 //framerate for DistorteD and up
#define PRELOAD_DELAY 200 //specifies the maximum keysound preload delay before the actual key in ms
#define ENDSONG_DELAY 3000 //end a song 3 seconds after the last measure

//BMS event/channel list

#define BMS_BPM_CUST 8
#define BMS_BPM 3
#define BMS_STOP 9

#define BMS_AUTOPLAY 1

#define BMS_P1_1 11
#define BMS_P1_2 12
#define BMS_P1_3 13
#define BMS_P1_4 14
#define BMS_P1_5 15
#define BMS_P1_6 18
#define BMS_P1_7 19
#define BMS_P1_S 16

#define BMS_P2_1 21
#define BMS_P2_2 22
#define BMS_P2_3 23
#define BMS_P2_4 24
#define BMS_P2_5 25
#define BMS_P2_6 28
#define BMS_P2_7 29
#define BMS_P2_S 26

//.1 file event type list

#define ONE_TYPE_BPM 4
#define ONE_DUMMY 0xF0
#define ONE_STOP 0xF1 //this is not the actual event, just a marker for later
#define ONE_LONG_P1 0xF2 //same here
#define ONE_LONG_P2 0xF3 //same here

#define ONE_AUTOPLAY 7

#define ONE_TYPE_P1 0
#define ONE_TYPE_P2 1

#define ONE_MEASURE 0xC

#define ONE_NOTECOUNT 0x10
#define ONE_BAD 0x10
#define ONE_MEASURESIZE 5

#define ONE_JUDGEMENT 8

#define ONE_ENDSONG 6
#define ENDSEQ 0x7FFFFFFF

#define ONE_KEYCHANGE1 2
#define ONE_KEYCHANGE2 3

#define EVENTSIZE 8



//we build a linked list of events so we can easily append stuff
typedef struct
{
	unsigned int time;
	byte type;
	byte data1;
	unsigned short int data2;

	void *prev;
	void *next;
} event;

//this will hold pointers to the first node for each chart
event *onecharts[CHARTCOUNT];

//These are from D.C.Fish (Gold AC)
//note to self: look up other charts
unsigned short int judgements[6] = {0xF0, 0xFA, 0xFF, 0x3, 0x8, 0x12};

//offsets inside the .1 header for chart i
int offsets[CHARTCOUNT] = {0, 8, 16, 24, 48, 56, 64};

//the write order inside the official .1 charts is h7, h14, n7, n14, a7, a14, b, maybe this matters
//anyway, it's not much hassle, so let's write them in "official" order
int worder[7] = {0, 4, 1, 5, 2, 6, 3};
int onesize; //size of the whole .1 file
byte *onefile;



//inserts an event into the chain for chart i while keeping the timestamps in ascending order
//this might seem overly complicated, but for several reasons we need to know where exactly
	//events with equal timestamps are positioned
void add_event(int i, event ev)
{
	event *temp, *ev_n;

	if(!onecharts[i])
	{
		onecharts[i] = malloc(sizeof(event));
		memcpy(onecharts[i], &ev, sizeof(event));
		onecharts[i]->prev = onecharts[i]->next = NULL;
		return;
	}

	temp = onecharts[i];

	while(temp->next != NULL && temp->time < ev.time)
	{
		temp = temp->next;
	}

	ev_n = malloc(sizeof(event)); //create the new event to insert
	memcpy(ev_n, &ev, sizeof(event)); //and copy what we need

	if(temp->time < ev.time) //this is an append or insert after the current note
	{
		if(temp->next)
			((event*)temp->next)->prev = ev_n;

		ev_n->next = temp->next;
		temp->next = ev_n;

		ev_n->prev = temp;

	}
	else  //this is an insert before the current node
	{
		if(temp->prev)
			((event*)temp->prev)->next = ev_n;
		else //this is to become the first element
			onecharts[i] = ev_n;

		ev_n->next = temp;
		ev_n->prev = temp->prev;

		temp->prev = ev_n;		
	}
}

//deletes an event from the chain
void delete_event(event *ev)
{
	event *pr, *nx;
	pr = ev->prev;
	nx = ev->next;

	if(pr)
		pr->next = nx;
	if(nx)
		nx->prev = pr;

	free(ev);
}

//converts the bms events to .1 events
//this does _not_ add the standard events yet
void convert_to_1_events()
{
	int i,j,k;
	char measure_c[4]; //use this to store the first 3 bytes after '#'. I hate writing parsers
	int measure,channel;
	float mtime;
	int denom, nume; //denominator and nominator of a note inside a measure

	byte ev_type;
	float ev_time;
	byte ev_data1;
	int bms_ev;
	event temp;

	int ln;

	unsigned short int ev_data2;

	char *data;

	measure_c[3] = '\0';

	temp.next = temp.prev = NULL;

	printf("converting bms charts to .1 charts\n");
	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(!bms[i].length) continue;

		printf("chart %s: ", CHARTNAMES[i]);

		data = bms[i].data;

		for(j = 1; j < bms[i].length-6; j++)
		{
			//find a candidate (parse for "#XXXYY:")
			if(data[j] == '#' && data[j+6] == ':')
			{
				measure_c[0] = data[j+1];
				measure_c[1] = data[j+2];
				measure_c[2] = data[j+3];
				sscanf(measure_c, "%d", &measure);
				sscanf(data+j+4, "%d", &channel);

				//check the channel and if we support it, if so set the event type and data type
				//don't be scared by the fancy switch magic, we just evaluate bms events to .1 events in a tricky way to save typing work :P
				ev_data1 = 0;
				ev_data2 = 0;
				ev_time = 0;
				ev_type = 0;

				if(channel >= 50) //longnotes
				{
					channel -= 40;
					ln = 1;
					//printf("!");
				}
				else
					ln = 0;

				switch(channel)
				{
					case BMS_P1_S:
						ev_data1 += 4;
					case BMS_P1_6:
					case BMS_P1_7:
						ev_data1 -= 2;
					case BMS_P1_1:
					case BMS_P1_2:
					case BMS_P1_3:
					case BMS_P1_4:
					case BMS_P1_5:
						ev_data1 += channel - 11;
						ev_type = ONE_TYPE_P1;
					break;

					case BMS_P2_S:
						ev_data1 += 4;
					case BMS_P2_6:
					case BMS_P2_7:
						ev_data1 -= 2;
					case BMS_P2_1:
					case BMS_P2_2:
					case BMS_P2_3:
					case BMS_P2_4:
					case BMS_P2_5:
						ev_data1 += channel - 21;
						ev_type = ONE_TYPE_P2;
					break;

					case BMS_BPM:
					case BMS_BPM_CUST:
						ev_type = ONE_TYPE_BPM;
					break;

					case BMS_AUTOPLAY:
						ev_type = ONE_AUTOPLAY;
					break;

					case BMS_STOP:
						ev_type = ONE_STOP;
					break;

					default:
						//printf("%d ", channel);
						continue; //unsupported event type, we skip this
				}

				//calculate the start of the measure
				mtime = 0;
				for(k = 0; k < measure; k++)
					mtime += bms[i].msize[k];

				//now that we've got the measure and the channel, count the number of events for this measure to get the denominator
				denom = 0;
				for(k = j+7/*after the ':'*/; (k<=bms[i].length) && (data[k] != '\n') && (data[k] != '\r') && (data[k] != '#'); k+=2) //just to be sure...
					denom++;

				k = j + 7;
				nume = 0;

				//now for the interesting part
				for(k = j+7; nume < denom; k+=2)
				{
					ev_time = mtime + ((float)nume/(float)denom)*bms[i].msize[measure];
					ev_time *= 4. * 60. * 1000./ bms[i].bpm; //timestamp in milliseconds
					nume++;

					bms_ev = strtoi(data+k);

					if(!bms_ev)
						continue; //'00', skip this

					if(channel == BMS_AUTOPLAY)
					{
						ev_data2 = bms[i].ref[bms_ev-1]; //remember that we always count from 0 while bms indices start from 1
					}
					else if(channel == BMS_BPM_CUST)
					{
						//printf("BPM change!\n");
						if(((bms[i].bpmchange[bms_ev-1] - (int)bms[i].bpmchange[bms_ev-1]) < 0.01) || (bms[i].bpmchange[bms_ev-1] > 655.35))
						{
							//an interesting way to work around the short int limit
							ev_data1 = 1;
							ev_data2 = (unsigned short int)(bms[i].bpmchange[bms_ev-1]);
						}
						else
						{
							ev_data1 = 100;
							ev_data2 = (unsigned short int)bms[i].bpmchange[bms_ev-1]*100;
						}
					}
					else if(channel == BMS_BPM)
					{
						//printf("BPM change!\n");
						ev_data1 = 1;
						ev_data2 = (unsigned short)strtoi_h(data+k); //in this case we have to use hex, meh
					}
					else if(channel == BMS_STOP)
					{
						//printf("STOP!\n");
						ev_data2 = (unsigned short)strtoi(data+k) - 1; //BMS starts indexing at 1 blablabla
						//continue;
					}
					else if(channel >= 11 && channel <= 29)
					{
						//for now we store the keysound number in the key event; we create the actual keysound event later
						ev_data2 = bms[i].ref[bms_ev-1]; //data2 is the keysound index inside the .2dx file

						if(bms_ev == bms[i].lnobj)
						{
							//longnote end marker; skip this to prevent useless notes
							//printf("!");
							continue;
						}

						if(channel <= 19) //1player side
						{
							bms[i].numkeys[0]++;

							if(ln) //longnote handling
								ev_type = ONE_LONG_P1; //we will have to decrease the notecount later because the longnote's endnote doesn't count
						}
						else //2player side
						{
							bms[i].numkeys[1]++;

							if(ln)
								ev_type = ONE_LONG_P2; 
						}
					}

					//now add the actual event to the list;

					temp.time = ev_time;
					temp.type = ev_type;
					temp.data1 = ev_data1;
					temp.data2 = ev_data2;
					//printf("time:%d channel:%d type:%d(bms:%d), data1:%d, data2:%d\n", (int)ev_time, channel, ev_type, bms_ev, ev_data1, bms[i].ref[bms_ev-1]);
					add_event(i, temp);
					if(i >= 4 && temp.type == ONE_TYPE_BPM) add_event(i, temp); //for some reason these are added twice for double charts
				}

			}
		}
		printf("keypresses: %d(P1) %d(P2)\n", bms[i].numkeys[0], bms[i].numkeys[1]);
		//printf("adding measure lines...\n");
		//now that we're done with the normal events, add the measure lines
		ev_time = 0;
		temp.time = 0;
		temp.type = ONE_MEASURE;
		temp.data1 = 0;
		temp.data2 = 0;
		for(j = 0; j <= bms[i].mcount; j++)
		{
			temp.time = ev_time * 4. * 60. * 1000. / bms[i].bpm;
			if(i >= 4) //for double charts, add a measure line for the 2p side, too
			{
				temp.data1 = 1;
				add_event(i, temp);
			}
			temp.data1 = 0;
			add_event(i, temp);
			ev_time += bms[i].msize[j];
		}

		//we add this dummy event as the marker for the end of the last measure
		//note that this is not really optimal for calculating the end of the song (will fix later)
		temp.type = ONE_DUMMY;
		temp.time = ev_time * 4. * 60. * 1000. / bms[i].bpm;
		add_event(i, temp);
	}

}

void add_standard_events()
{
	int i,j,k;

	event total_notes_p1, total_notes_p2;
	event bpm; //starting bpm
	event msize; //measure size
	event timings; //timing windows
	event endmark; //marks the end of the song
	event endseq; //end sequence
	event *temp;
	int twop = 0; //double chart indicator

	printf("adding standard events...");
	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(!onecharts[i])
			continue;

		if(i >= 4) twop = 1;
		else twop = 0;

		printf("%s...", CHARTNAMES[i]);

		for(k = twop; k >= 0; k--) //double charts have two entries for everything, so add them in, too
		{

		memset(&total_notes_p1, 0, sizeof(event));
		memset(&total_notes_p2, 0, sizeof(event));
		memset(&bpm, 0, sizeof(event));
		memset(&msize, 0, sizeof(event));
		memset(&timings, 0, sizeof(event));
		memset(&endmark, 0, sizeof(event));
		memset(&endseq, 0, sizeof(event));

		
		total_notes_p1.type = ONE_NOTECOUNT;
		total_notes_p1.data2 = bms[i].numkeys[0];

		total_notes_p2.type = ONE_NOTECOUNT;
		total_notes_p2.data1 = 1;
		total_notes_p2.data2 = bms[i].numkeys[1];

		bpm.type = ONE_TYPE_BPM;
		if((bms[i].bpm-(int)bms[i].bpm < 0.01) || (bms[i].bpm > 655.35)) //ass-check for fractional bpm, probably doesn't even matter
		{
			bpm.data1 = 1;
			bpm.data2 = (unsigned short int)(bms[i].bpm);
		}
		else
		{
			bpm.data1 = 100;
			bpm.data2 = (unsigned short int)bms[i].bpm*100;
		}

		msize.type = ONE_MEASURESIZE;
		msize.data1 = 4;
		msize.data2 = 4;

		//add judgement events
		timings.type = ONE_JUDGEMENT;
		for(j = 5; j >= 0; j--)
		{
			timings.data1 = (byte)j;
			timings.data2 = judgements[j];
			add_event(i, timings);
		}

		add_event(i, msize);
		add_event(i, bpm);
		if(k != 1)
		{
			add_event(i, total_notes_p2);
			add_event(i, total_notes_p1);
		}


		temp = onecharts[i];
		while(temp->next) temp = temp->next; //temp->type == ONE_DUMMY

		endmark.type = ONE_ENDSONG;
		endmark.time = temp->time + ENDSONG_DELAY*(1-twop*(1-k)); //evil hack :D; ensures we add both the p1 and the p2 event at the same time
		endmark.data1 = k;

		add_event(i, endmark);

		}

		endseq.time = ENDSEQ;
		add_event(i, endseq);

		//kill the dummy event
		temp = onecharts[i];
		while(temp = temp->next)
		{
			if(temp->type == ONE_DUMMY)
			{
				delete_event(temp);
				break; //only one dummy event per chart
			}
		}
	}
	PNEWLINE;
}

void handle_longnotes()
{
	int i, player, time, count;

	event *temp;
	event *temp2;

	printf("processing longnotes (if neccesary)..");	
	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(!onecharts[i])
			continue;

		printf("%s..", CHARTNAMES[i]);

		temp = onecharts[i];
		count = 0;

		//find longnote events
		do 
		{
			if(temp->type == ONE_LONG_P1 || temp->type == ONE_LONG_P2) //longnote
			{
				count++;
				if(temp->type == ONE_LONG_P1)
					player = ONE_TYPE_P1;
				else
					player = ONE_TYPE_P2;

				//find the exact time the longnote ends
				temp2 = temp;
				while(temp2 = temp2->next)
				{
					if(temp2->type == temp->type && temp2->data1 == temp->data1) //end the longnote
					{
						time = temp2->time;

						delete_event(temp2); //kill the endnote
						bms[i].numkeys[player]--;
						break;
					}
				}

				//start again; this time delete all keypresses on the same lane during and _at the end_ of the longnote
				temp2 = temp;
				while(temp2 = temp2->next)
				{
					if(temp2->time > time)
						break;

					if(temp2->type == player && temp2->data1 == temp->data1) //same player, same lane
					{
						temp2 = temp2->prev;
						delete_event(temp->next); //kill additional keypresses during a longnote
						bms[i].numkeys[player]--;
					}
				}
			}
		} while(temp = temp->next);
		printf("(%d)...", count);
	}
	PNEWLINE;
}

//remove multiple keysound loads
void cleanup_1()
{
	int i,j,k;

	event *temp;
	event *temp2;
	event ref;

	printf("cleaning up charts..");	
	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(!onecharts[i])
			continue;

		printf("%s..", CHARTNAMES[i]);
		memset(&ref, 0, sizeof(event));

		//delete useless keysound loads; walk the chain once for the p1 side, once for the p2 side
		for(k = 0; k <= 1; k++)
		{
			for(j = 0; j <= 7; j++)
			{
				ref.data2 = 0xFFFF; //just to be sure we don't accidently kill the first sample load
				temp = onecharts[i];

				do 
				{
					if(temp->type == k+2 && temp->data1 == j) //soundchange for player k and key j
					{
						if(temp->data2 == ref.data2)
						{
							//printf("deleted a keysound event in %s: player:%d time:%d key:%d sound:%d\n", \
								CHARTNAMES[i], k+1, temp->time, temp->data1, temp->data2);
							temp2 = temp->next;
							delete_event(temp);
							temp = temp2;

							if(!temp) break;
						}
						else
						{
							ref.data2 = temp->data2;
						}
					}
				} while(temp = temp->next);
			}
		}
	}
	PNEWLINE;
}

void create_1()
{
	int i;
	int pos; //position
	int size[CHARTCOUNT]; //size of the chains inside the .1 file

	event *temp;

	pos = 0x60; //start writing after the .1 header which is 96 bytes
	memset(size, 0, sizeof(int)*CHARTCOUNT);

	onesize = 96; //header is always the same size

	printf("creating .1 file\n");
	//first calculate the size for each event chain and the resulting size of the .1 file
	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(!onecharts[i])
			continue;

		temp = onecharts[i];

		do
		{
			//printf("(%d %d) ", temp->time, temp->type);
			size[i] += EVENTSIZE;
		}
		while(temp = temp->next);

		onesize += size[i];
		//printf("size for %s is %d\n", CHARTNAMES[i], size[i]);
	}


	onefile = malloc(onesize);
	memset(onefile, 0, onesize);

	//printf("onesize is %d\n", onesize);

	//write the header and the chains to the buffer
	for(i = 0; i < CHARTCOUNT; i++)
	{
		//printf("size[worder[i]]: %d\n", size[worder[i]]);
		if(!onecharts[worder[i]])
			continue;


		memcpy(onefile+offsets[worder[i]], &pos, 4); //write the position of the chart
		memcpy(onefile+offsets[worder[i]]+4, &size[worder[i]], 4); //write the size of the chart

		//write out the events
		temp = onecharts[worder[i]];
		do
		{
			memcpy(onefile+pos, &temp->time, 4);
			memcpy(onefile+pos+4, &temp->type, 1);
			memcpy(onefile+pos+5, &temp->data1, 1);
			memcpy(onefile+pos+6, &temp->data2, 2);

			pos += 8;
		}
		while(temp = temp->next);

	}
	PNEWLINE;
}

void adjust_bpm()
{
	int i;

	float oldbpm = 0;
	float newbpm = 0;
	int stime = 0;
	int tdiff = 0;
	event *temp;

	event *tadjust;

	printf("adjusting for (possible) BPM changes...");	

	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(!onecharts[i]) continue;

		temp = onecharts[i];

		oldbpm = bms[i].bpm;

		//search for bpm change events and adjust all timestamps after those events accordingly
		do
		{
			if(temp->type == ONE_TYPE_BPM)
			{
				newbpm = temp->data2 / temp->data1;
				stime = temp->time;

				if(oldbpm == newbpm) //no need to do anything
					continue;
				
				tadjust = temp;

				//events are sorted by time so this works just peachy
				while(tadjust = tadjust->next)
				{
					tdiff = tadjust->time - stime;
					tdiff = (int)((float)tdiff * oldbpm / newbpm); //adjust the timestamp accordingly
					tadjust->time = stime + tdiff;
				}

				oldbpm = newbpm;
			}
		}
		while(temp = temp->next);
	}
	PNEWLINE;
}

void writeout_1()
{
	FILE *f;

	printf("writing .1 file to disk\n");
	f = fopen("output.1", "wb");
	fwrite(onefile, onesize, 1, f);
	fclose(f);
}


void add_keysounds()
{
	event *key;
	event *p;
	event temp;
	int i;
	int tdiff;

	printf("adding keysound events...");
	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(!onecharts[i]) continue;

		printf("%s...", CHARTNAMES[i]);

		//search the chain for key events
		key = onecharts[i];
		do
		{
			if(key->type == ONE_TYPE_P1 || key->type == ONE_TYPE_P2)
			{
				//found a key event, let's add the keysound change for it
				tdiff = 0;

				//walk the timeline backwards and try to find the previous key event for the same key
				p = key;
				while(p = p->prev)
				{
					if(p->type == key->type && p->data1 == key->data1)
					{
						tdiff = p->time;
						break;
					}
				}

				//calculate the time difference between the two key events and place the keysound event right between them
				
				tdiff = key->time - tdiff;

				if(tdiff == key->time && key->time < PRELOAD_DELAY) //we found no key event before this one and the current key event is really close to time 0
					; //do nothing as tdiff == key->time
				else if(tdiff >= PRELOAD_DELAY*2) //the keys are far enough apart to use the standard preload delay
					tdiff = PRELOAD_DELAY;
				else //the keys are quite close together so place the keysound load event right between them
				{
					tdiff /= 2;
					//printf("TDIFF: %d\n", tdiff);
				}

				memcpy(&temp, key, sizeof(event));
				temp.time -= tdiff;
				temp.type += 2; //preload event = playerside + 2

				//printf("key: time:%d player:%d key:%d index:%d TDIFF:%d\n", key->time, key->type-2, key->data1+1, key->data2, tdiff);
				//printf("keysoundevent: time:%d player:%d key:%d index:%d TDIFF:%d\n", temp.time, temp.type-2, temp.data1+1, temp.data2, tdiff);
				add_event(i, temp);

				key->data2 = 0;
			}
		}
		while(key = key->next);
	}
	PNEWLINE;
}

void add_stop_events()
{
	int i;

	float bpm = 0;

	float tdiff = 0;
	int d1, d2;
	event *temp;
	event *helper;
	event bpm_help;

	printf("adjusting for (possible) stops...");	

	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(!onecharts[i]) continue;

		if(!bms[i].stopcount) continue;

		printf("%s...", CHARTNAMES[i]);

		temp = onecharts[i];

		//search for stop events and adjust all timestamps after those events accordingly
		do
		{
			if(temp->type == ONE_STOP)
			{
				//find the bpm for the current stop event
				helper = onecharts[i];
				do
				{
					if(helper->type == ONE_TYPE_BPM)
					{
						d1 = helper->data1;
						d2 = helper->data2;
						bpm = (float)helper->data2 / (float)helper->data1;
					}
				} while((helper = helper->next) && (helper->time <= temp->time));

				//calculate the stop time
				tdiff = bms[i].stops[temp->data2] / 192.; 
				tdiff = tdiff * 4. * 60. * 1000. / bpm;

				//adjust all event times after the stop event
				helper = temp->next;
				do
				{
					if(helper->time > temp->time && helper->time != ENDSEQ) //in case there's another event at the same time as the stop
						helper->time += tdiff;
				}
				while(helper = helper->next);

				//add two bpm changes to simulate the stop
				bpm_help.time = temp->time;
				bpm_help.type = ONE_TYPE_BPM;
				bpm_help.data1 = 100;
				bpm_help.data2 = 100; //1 bpm (stop); iidx doesn't seem to support any lower

				add_event(i, bpm_help);
				if(i >= 4) //as always, add another event for double charts
					add_event(i, bpm_help);

				bpm_help.time += tdiff;
				bpm_help.data1 = d1;
				bpm_help.data2 = d2; //restore old bpm

				add_event(i, bpm_help);
				if(i >= 4)
					add_event(i, bpm_help);
			}
		}
		while(temp = temp->next);

		//delete all stop events

		temp = onecharts[i];

		do
		{
			helper = temp->next;
			if(helper && helper->type == ONE_STOP)
				delete_event(helper);
		}
		while(temp = temp->next);
	}
	PNEWLINE;
}

void stretch_times(double factor)
{
	int i;
	event *temp;

	for(i = 0; i < CHARTCOUNT; i++)
	{
		if(!onecharts[i]) continue;

		temp = onecharts[i];
		//walk the chain and multiply timestamps with factor
		do
			if(temp->time != ENDSEQ)
				temp->time = (int)((float)temp->time * factor);
		while(temp = temp->next);
	}
}
