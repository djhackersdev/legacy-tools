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
	int offset;
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

byte cryptkey[] = { //empress
     0x28, 0x22, 0x28, 0x54, 0x63, 0x3F, 0x0E, 0x42,
     0x6F, 0x45, 0x4E, 0x50, 0x67, 0x53, 0x61, 0x7C,
     0x04, 0x46, 0x00, 0x3B, 0x13, 0x2B, 0x45, 0x6A };

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

		block_xor(cur, cur-8);
		block_xor(cur, cryptkey+16);
		block_obfus(cur);
		block_xor(cur, cryptkey+8);
		block_swap(cur);
		block_obfus(cur);
		block_xor(cur, cryptkey);
        }

	memcpy(data, "%iO0", 4); //encryption identifier
	enclen -= 8;
	memcpy(data+4, &enclen, 4); //write the length of the decoded 2dx file
	return data;
}	

int main(int argc, const char **argv)
{
	wavefile waves[MAXWAVS];

	FILE *wavfile;
	FILE *out;
	FILE *bmsfile;

	byte *decrypted, *encrypted, *bms;

	int wav = 0; //wavefile count
	int pos = 0, i = 0, ii = 0, j, firstwav = 0, bmslength = 0, len = 0;
	int length = 72; //use this to compute the final file length along the way (72 = 2dx headersize)
	int prev, maxwav;

	char wavepath[32]; //helperstring; ugly but works

	int bmestyle = 0;
	memset(waves, 0, MAXWAVS*sizeof(wavefile));

	printf("reading samples...");
	for(i = 1; i <= MAXWAVS; i++) //this is really inefficient but works, meh
	{
		if(bmestyle == 0)
		{
			sprintf(wavepath, "%s.wav", itostr(i));
		}
		else if(bmestyle == 1) 
		{
			sprintf(wavepath, "%d.wav", i);
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
			maxwav = i;
		}
	}

	if(wav == 0) //learn2placefilesindirectories
	{
		printf("no wave files found, go die in a fire\n");
		return 0;
	}
	printf("OK\n");
	printf("\ntotal samplecount: %d, actual wavcount in .2dx: %d\n\n",wav, maxwav);

	wav = maxwav;

	for(i = maxwav-2; i >= 0; i--)
	{
		if(waves[i].length == 0)
		{
			waves[i].data = waves[i+1].data;
			waves[i].length = waves[i+1].length;
			printf("added wav: %i, len %i\n", i, waves[i].length);
			length += waves[i].length;
		}
	}


	//start constructing the 2dx file
	decrypted = malloc(length);
	memset(decrypted, 0, length);

	firstwav = 72+wav*4; //ugly...
	memcpy(decrypted + 16, &firstwav, 4); //first wave position = 2dx header + size of TOC (= 72 + wavecount * sizeof(int))
	memcpy(decrypted + 20, &wav, 4); //the total numbers of waves

	printf("writing samples...");
	pos = firstwav;
	for(i = 0; i < maxwav; i++)
	{
		memcpy(decrypted+pos, aftertoc, 8); //"2DX9" + headersize (== 24)
		memcpy(decrypted+pos+8, &waves[i].length, 4); //size of the wave
		memcpy(decrypted+pos+12, aftertoc+12, 12); //rest of the header
		memcpy(decrypted+pos+24, waves[i].data, waves[i].length); //write the actual wav
		memcpy(decrypted+0x48+(i*4), &pos, 4); //write toc entry
		pos += 24 + waves[i].length;
	}
	printf("OK\n");

	printf("encrypting...");
	encrypted = encrypt(decrypted, length);
	printf("OK\n");

	out = fopen("output.2dx", "wb"); //open the 2dx file and write out the array
	fwrite(encrypted, 8+CIELMOD8(length), 1, out);
	fclose(out);

	return 0;
}
