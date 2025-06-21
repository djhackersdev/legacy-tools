//crack2dx_new.c
//decrypts .2dx files from every style from 9th to Troopers
//decryption method and keys taken from crack2dx.c, rewritten with added support for Troopers, Empress and usablility
//...and for bugfixing -_-

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define byte unsigned char

byte keys_09[] = {  //9th style
     0x97, 0x1E, 0x24, 0xA0, 0x9A, 0x00, 0x10, 0x2B,
     0x91, 0xEF, 0xD7, 0x7A, 0xCD, 0x11, 0xAF, 0xAF,
     0x8D, 0x26, 0x5D, 0xBB, 0xE0, 0xC6, 0x1B, 0x2B };

byte keys_10[] = { //10th style
     0x2D, 0x86, 0x56, 0x62, 0xD7, 0xFD, 0xCA, 0xA4,
     0xB3, 0x24, 0x60, 0x26, 0x24, 0x81, 0xDB, 0xC2,
     0x57, 0xB1, 0x74, 0x6F, 0xA7, 0x52, 0x99, 0x21 };

byte keys_11[] = { //Happy Sky+
     0xED, 0xF0, 0x9C, 0x90, 0x44, 0x1A, 0x5A, 0x03,
     0xAB, 0x07, 0xC1, 0x99, 0x23, 0x24, 0x32, 0xC7,
     0x5F, 0x32, 0xA5, 0x97, 0xAD, 0x98, 0x0F, 0x8F };

byte keys_16[] = { //empress
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


int main(int argc, byte **argv)
{
	int i;
	int style=2;
	int mult;
	byte *buf, *buf_dec, *enc;
	byte *cur, *prev;
	byte *cryptkey;
	int *wavcount;
	int *wavpos, *wavlen;
	int len;
	FILE *in, *out;
	unsigned char n_out[10]; //wav name

	printf("oldac2newac_2dx by v0.1 ryuuou\n\treencrypts .2dx files from Troopers AC and before \n\tto .2dx compatible with Empress AC\n\n");

	if(argc < 2)
	{
		printf("USAGE: oldac2newac_2dx <file>\n");
		return 0;
	}
	
	printf("\nprocessing %s...\n", argv[1]);

	in = fopen(argv[1], "rb");

	if(!in)
	{
		printf("not found: %s\n", argv[1]);
		return -1;		
	}

	fseek(in, 0, SEEK_END);
	len = (int)ftell(in);
	fseek(in, 0, SEEK_SET);
		
	buf = malloc(len);

	fread(buf, len, 1, in);
	fclose(in);

	//identify iidx version

	printf("identifying encryption...");
	mult = 1;
	if(!strncmp(buf, "%eNc", 4))
	{
		printf("9th style\n");
		cryptkey = keys_09; //9th style
	}
	else if(!strncmp(buf, "%e10", 4))
	{
		printf("10th style\n");
		cryptkey = keys_10; //10th style
	}
	else if(!strncmp(buf, "%e11", 4))
	{
		printf("RED\n");
		cryptkey = keys_11; //RED
	}
	else if(!strncmp(buf, "%e12", 4))
	{
		printf("Happy Sky/DistorteD/GOLD\n");
		cryptkey = keys_11; //HS+
		mult = 0;
	}
	else if(!strncmp(buf, "%hid", 4))
	{
		printf("Troopers\n");	
		cryptkey = keys_11; //Troopers
		mult = 0;
	}
	else if(!strncmp(buf, "%iO0", 4))
	{
		printf("Empress, nothing to do\n");	
		cryptkey = keys_16; //Empress
		return 0;
	}
	else
	{
		printf("unknown encryption, trying unencrypted...\n");
		mult = 2;
	}


	//decrypt



	if(!(mult == 2)) //skip for unencrypted
	{
		buf_dec = malloc(len);
		memcpy(buf_dec, buf, len); //copy the encrypted part
		memset(buf, 0, 8); //zero the first block to make the loop easier

		for(i = 0; i <= len-16; i+= 8)
		{
			cur = buf_dec + 8 + i; //cur points to the current encrypted block to decrypt
			prev = buf + i; //prev points to the cur-8 in encrypted form

			block_xor(cur, cryptkey);
			block_obfus(cur);
			block_swap(cur);

			if(mult) // for versions prior to DistorteD and Empress
			{
				block_xor(cur, cryptkey+8);
				block_obfus(cur);
				block_xor(cur, cryptkey+16);
			}

			block_xor(cur, prev);
		}
	}
	else
	{
		buf_dec = malloc(len);
		memcpy(buf_dec, buf, len); //copy for splitting
	}


	//reencrypt to empress

	cryptkey = keys_16;

	for(i = 0; i <= len-16; i+= 8)
	{
		cur = buf_dec + i + 8; //cur points to the current decrypted block to encrypt

		block_xor(cur, cur-8);
		block_xor(cur, cryptkey+16);
		block_obfus(cur);
		block_xor(cur, cryptkey+8);
		block_swap(cur);
		block_obfus(cur);
		block_xor(cur, cryptkey);
	}

	memcpy(buf_dec, "%iO0", 4);

	//*((int*)buf_dec+4) = len-8;

	out = fopen(argv[1], "wb");
	fwrite(buf_dec, len, 1, out);
	fclose(out);

	printf("done\n");
	free(buf);
	free(buf_dec);
	return 0;
}
