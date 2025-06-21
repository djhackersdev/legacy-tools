//create2dx_dumb.c by ryuuou
//Authoring the 2dx file is based on create2dx.c by Kitaru
//Encrypting is taking the reverse way from crack2dx.c by some unknown author (at least to me)
//Packs BME-style sequenced .wav (01.wav to ZZ.wav, but should work with BMS-style 01 - FF as well) in the current folder
//into an unencrypted unencrypted.2dx and an DistorteD-style encrypted encrypted.2dx
//Takes no arguments and always outputs to the same files; only works with uppercase on unix but should work with lowercase on windows;
//We also assume that the ascending order or the wavefiles equals their order in the corresponding bme/bms file
//...hence "_dumb"


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//wav files from 01-0A-0Z-10-ZZ (you get it) = 36*36 - 1
#define MAXWAVS (36*36-1)

#define CIELMOD8(a) (!(a%8)?(a):((a)+(8-(a)%8)))

typedef unsigned char byte;

typedef struct{
	int length;
	byte *data;
} wavefile;

const byte aftertoc[24] = {
   	'2', 'D', 'X', '9', //const string
	0x18, 0x00, 0x00, 0x00, //headersize (== 24)
	0x00, 0x00, 0x00, 0x00, //size of wave file (written at runtime)
	0x30, 0x32, 0xFF, 0xFF, //panning/initial volume?
	0x40, 0x00, 0x08, 0x00, //??? (64, 0, 8, 0)
	0x00, 0x00, 0x00, 0x00  //??? (0, 0, 0, 0)
};


//helper function for BME numbering weirdness;
//converts a number to a 01-ZZ style string
char *itostr(int i)
{
	char *str;

	str = malloc(3);

	str[0] = i / 36; //works because this floors
	if(str[0] > 9)
		str[0] += 7; //there's a gap of 8 characters between '9' and 'A'
	str[0] += 48; //fancy charactermap magic

	str[1] = i % 36; //mod operator = <3
	if(str[1] > 9)
		str[1] += 7; 
	str[1] += 48;

	str[2] = '\0';

	return str;
}

///
/// encryption related
///

byte cryptkey[8] = { 0xED, 0xF0, 0x9C, 0x90, 0x44, 0x1A, 0x5A, 0x03 };

//XORs two 8 byte blocks
void block_xor(byte *data, byte *parm)
{
	int i;
	for (i = 0 ; i < 8 ; i++) 
		data[i] ^= parm[i];
	return;
}

//Swaps the lower 4 bytes with the upper 4 bytes
void block_swap(byte *data)
{
	int i;
	for (i = 0 ; i < 4 ; i++)
	{
		data[i]   ^= data[i+4]; //let's do this without temp variables ;D
		data[i+4] ^= data[i];
		data[i]   ^= data[i+4];
	}
	return;
}

//I bow to whoever figured this one out
void block_obfus(byte *data)
{
	byte a,b,c,d,e,f,g,h,i;

	a = data[0] * 63;
	b = a + data[3];
	c = data[1] * 17;
	d = c + data[2];
	e = d + b;
	f = e * data[3];
	g = f + b + 51;
	h = b ^ d;
	i = g ^ e;

	data[4] ^= h;
	data[5] ^= d;
	data[6] ^= i;
	data[7] ^= g;

	return;
}

//Encrypts length bytes of inc and returns a pointer to the encoded array
byte *encrypt(byte *inc, int length)
{
	byte *data;
	byte *cur; //current block of 8 bytes

	int i, enclen;

	enclen = 8 + CIELMOD8(length); //let's just make sure we're not running into uninitialized memory while encrypting 
	data = malloc(enclen);

	memset(data, 0, enclen); 
	memcpy(data+8, inc, length); //fill up the encoded array with unencoded stuff


	for(i = 8; i <= enclen-8; i+=8) //encode in blocks of 8 bytes, start at byte 8 of data
	{
		cur = data+i;

		//Step 1: XOR the current block with the encrypted previous block
		block_xor(cur, cur-8);

		//Step 2: Swap the lower and upper 4 bytes of the block
		block_swap(cur);

		//Step 3: Further obfuscation with XOR; the lower 4 bytes are untouched, so encrypting works the same as decrypting
		block_obfus(cur);

		//Step 4: XOR the block (yet again) with DistorteD's key
		block_xor(cur, cryptkey);
        }

	memcpy(data, "%e12", 4); //encryption identifier
	enclen -= 8;
	memcpy(data+4, &enclen, 4); //write the length of the decoded 2dx file
	return data;
}


//searches *bms for 'WAVxy' where xy is the bme index (01-ZZ) indicated by index
//WARNING: this is very intolerant concerning capitalization and trailing whitespaces!
char *wavfrombms(byte *bms, int length, int index)
{
	char *bmeindex;
	int i = 0, j = 0;
	char *wavname;

	bmeindex = itostr(index);

	while(i < length - 5)
	{
		if(bms[i] == 'W' && bms[i+1] == 'A' && bms[i+2] == 'V' && bms[i+3] == bmeindex[0] && bms[i+4] == bmeindex[1]) //long if-clause is long
		{
			i += 6; //skip to the filename (after "WAVxy ")
			while((bms[i+j] != '\n') && (bms[i+j] != '\r') && i+j < length) //CR or LF
				j++; //count the length of the filename
			break; //we found what we wanted
		}

		i++;
	}


	wavname = malloc(j + 1);

	memcpy(wavname, bms+i, j); //copy the wavename

	wavname[j] = '\0'; //terminate the string

	return wavname;
}
	

int main(int argc, const char **argv)
{
	//coding while listening to kachoufuusetsu long, hell yeah!

	wavefile waves[MAXWAVS]; //use this to hold the wavefiles; MAYBE I SHOULD NAME THIS SMALL_WAVES ZOMG GET IT?

	FILE *wavfile;
	FILE *out;
	FILE *bmsfile;
	byte *decrypted, *encrypted, *bms;

	int wav = 0; //wavefile count
	int pos = 0, i = 0, ii = 0, firstwav = 0, bmslength = 0;
	int length = 72; //use this to compute the final file length along the way (72 = 2dx headersize)

	char wavepath[32]; //helperstring; ugly but works

	int bmestyle = 0;

	if(argc >= 2) //we got a parameter
	{
		bmestyle = 2;
		bmsfile = fopen(argv[1], "rb");
		
		if(bmsfile == NULL)
		{
			printf("file not found: %s\n", argv[1]);
			return 0;
		}

		fseek(bmsfile, 0, SEEK_END);
   		bmslength = (int)ftell(bmsfile);
		fseek(bmsfile, 0, SEEK_SET);
		
		bms = malloc(bmslength);

		fread(bms, bmslength, 1, bmsfile);
		
		fclose(bmsfile);
		printf("got file input (%s) of length %d\n", argv[1], bmslength);
	}


	memset(waves, 0, MAXWAVS*sizeof(wavefile));

	i = 1; //BME starts counting at 01.wav
	printf("reading samples...");
	while(i <= MAXWAVS) //this is really inefficient but works, meh
	{
		if(bmestyle == 0)
		{
			sprintf(wavepath, "%s.wav", itostr(i));
		}
		else if(bmestyle == 1) 
		{
			sprintf(wavepath, "%d.wav", i);
		}
		else if(bmestyle == 2)
		{
			sprintf(wavepath, "%s", wavfrombms(bms, bmslength, i));
			if(wavepath[0] == '\0')
			{
				i++;
				continue; //parsing the bms returned no file for index i
			}
		}

		wavfile = fopen(wavepath, "rb");

		if (wavfile != NULL)
		{
			//open the wave file and save its length and contents to the array
			printf("%s...",wavepath);
			fseek(wavfile, 0, SEEK_END);
   		 	waves[i-1].length = (int)ftell(wavfile);
			fseek(wavfile, 0, SEEK_SET);

			waves[i-1].data = malloc(waves[i-1].length);

			fread(waves[i-1].data, waves[i-1].length, 1, wavfile);

			fclose(wavfile);
			wav++;
			length += 4 + 24 + waves[i-1].length; //toc index + header + actual length
		}
		else
			if(bmestyle == 2)
				printf("\n!!%s referenced by bms file but not found!!\n...", wavepath);
		i++;
		//if(wav == 99) break;
	}

	if(wav == 0) //learn2placefilesindirectories
	{
		printf("no wave files found, go die in a fire\n");
		return 0;
	}
	printf("OK\n");
	printf("\ntotal samplecount: %d\n\n",wav);


	//start constructing the 2dx file
	decrypted = malloc(length);
	memset(decrypted, 0, length);

	firstwav = 72+wav*4; //ugly...
	memcpy(decrypted + 16, &firstwav, 4); //first wave position = 2dx header + size of TOC (= 72 + wavecount * sizeof(int))
	memcpy(decrypted + 20, &wav, 4); //the total numbers of waves

	ii = 0x48; //current position in the array

	//write the table of contents (absolute position of the waves in the .2dx file)
	pos = 0x48 + ((wav) * 4); //start of the wavefiles
	printf("writing TOC...");
	for(i = 0; i < MAXWAVS; i++)
	{
		if(waves[i].length > 0) //we only want waves with content
		{
			memcpy(decrypted+ii, &pos, 4);
			pos += waves[i].length + 0x18; //don't forget about the 24 byte header
			ii += 4;
		}
	}

	//write the actual waves
	printf("writing samples...");
	for(i = 0; i < MAXWAVS; i++)
	{
		if(waves[i].length > 0)
		{
			memcpy(decrypted+ii, aftertoc, 8); //"2DX9" + headersize (== 24)
			memcpy(decrypted+ii+8, &waves[i].length, 4); //size of the wave
			memcpy(decrypted+ii+12, aftertoc+12, 12); //rest of the header
			memcpy(decrypted+ii+24, waves[i].data, waves[i].length); //write the actual wave
			ii += 24 + waves[i].length;
		}
	}
	printf("OK\n");

	printf("encrypting...");
	encrypted = encrypt(decrypted, length);
	printf("OK\n");

	out = fopen("unencrypted.2dx", "wb"); //open the 2dx file and write out the array
	fwrite(decrypted, length, 1, out);
	fclose(out);

	out = fopen("encrypted.2dx", "wb"); //open the 2dx file and write out the array
	fwrite(encrypted, 8+CIELMOD8(length), 1, out);
	fclose(out);

	return 0;
}
