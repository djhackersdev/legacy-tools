//2dx.h
//deals with compiling the .2dx file

#define TWODXHSIZE 72 //size of the .2dx header
#define WAVHSIZE 24 //size of the wav header

int twodxsize;

byte *tdx; //this will hold the decrypted .2dx file
byte *tdx_enc; //this will hold the encrypted .2dx file

const byte wavheader[24] = {
   	'2', 'D', 'X', '9', //const string
	0x18, 0x00, 0x00, 0x00, //headersize (== 24)
	0x00, 0x00, 0x00, 0x00, //size of wave file (written at runtime)
	0x30, 0x32, 0xFF, 0xFF, //panning/initial volume?
	0x40, 0x00, 0x01, 0x00, //??? (64, 0, sample format, 0)
	0x00, 0x00, 0x00, 0x00  //??? (0, 0, 0, 0)
};
//sample format notes:
//1: 44.1khz, 16bit MS ADPCM stereo
//0: 44.1khz, 16bit MS ADPCM mono
//8: almost but not the same as 1?
//7: ???

//construct the .2dx file
//specs were taken from Kitaru's create2dx.c (QQQ)
void create_2dx()
{
	int i, j, firstwav;

	printf("authoring .2dx file\n");
	//calculate the size of the final .2dx file
	twodxsize = TWODXHSIZE + wavcount * WAVHSIZE + wavcount*4; //header + ToC + headers for wav files
	for(i = 0; i < wavcount; i++)
		twodxsize+=waves[i].length;

	twodxsize = CIELMOD8(twodxsize); //just make sure we don't run into unallocated memory while encrypting

	tdx = malloc(twodxsize);
	memset(tdx, 0, twodxsize);

	//twodxsize = oldsize;
	firstwav = TWODXHSIZE + wavcount*4;

	memcpy(tdx + 16, &firstwav, 4); //first wave position = 2dx header + size of TOC (= 72 + wavcount * sizeof(int))
	memcpy(tdx + 20, &wavcount, 4); //the total numbers of waves

	//write the ToC
	printf("writing sample TOC...", firstwav);
	j = firstwav;
	for(i = 0; i < wavcount; i++)
	{
		//printf("%d...", j);
		memcpy(tdx+TWODXHSIZE+i*4, &j, 4);
		j += waves[i].length + WAVHSIZE;
	}

	//write the waves
	printf("writing keysounds (%d in all)...\n", wavcount);
	j = firstwav;
	for(i = 0; i < wavcount; i++)
	{
		//printf("%d...", waves[i].length);
		memcpy(tdx+j, wavheader, 24); //write the header
		memcpy(tdx+j+8, &waves[i].length, 4); //write the length
		memcpy(tdx+j+24, waves[i].data, waves[i].length);
		j += WAVHSIZE + waves[i].length;
	}
	//printf("twodxsize: %d\n", twodxsize);
	//PNEWLINE;
}


//actually write the stuff to disk
void writeout_2dx()
{
	FILE *out;

	printf("writing output.2dx to disk\n");

	/*out = fopen("decrypted.2dx", "wb");

	//fflush(out);

	fwrite(tdx, twodxsize, 1, out);

	fclose(out);*/

	out = fopen("output.2dx", "wb");
	fwrite(tdx_enc, twodxsize+8, 1, out);
	fclose(out);

}
