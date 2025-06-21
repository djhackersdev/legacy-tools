//encrypt2dx.c by ryuuou
//encrypts a file with DistorteD's .2dx encryption algorithm
//takes the inverse way of crack2dx.c


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define CIELMOD8(a) (!(a%8)?(a):((a)+(8-(a)%8)))

typedef unsigned char byte;


byte keys_09[] = {  //9th style
     0x97, 0x1E, 0x24, 0xA0, 0x9A, 0x00, 0x10, 0x2B,
     0x91, 0xEF, 0xD7, 0x7A, 0xCD, 0x11, 0xAF, 0xAF,
     0x8D, 0x26, 0x5D, 0xBB, 0xE0, 0xC6, 0x1B, 0x2B };

byte keys_10[] = { //10th style
     0x2D, 0x86, 0x56, 0x62, 0xD7, 0xFD, 0xCA, 0xA4,
     0xB3, 0x24, 0x60, 0x26, 0x24, 0x81, 0xDB, 0xC2,
     0x57, 0xB1, 0x74, 0x6F, 0xA7, 0x52, 0x99, 0x21 };

byte keys_11[] = { //distorted+
     0xED, 0xF0, 0x9C, 0x90, 0x44, 0x1A, 0x5A, 0x03,
     0xAB, 0x07, 0xC1, 0x99, 0x23, 0x24, 0x32, 0xC7,
     0x5F, 0x32, 0xA5, 0x97, 0xAD, 0x98, 0x0F, 0x8F };

const char* ident[] = { "%hid", "%e12", "%e11", "%e10", "%eNc" };

void block_xor(byte *data, byte *parm)
{
	int i;
	for (i = 0 ; i < 8 ; i++) 
		data[i] ^= parm[i];
	return;
}


void block_swap(byte *data)
{
	int i;
	for (i = 0 ; i < 4 ; i++)
	{
		data[i]   ^= data[i+4];
		data[i+4] ^= data[i];
		data[i]   ^= data[i+4];
	}
	return;
}


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


byte *encrypt(byte *inc, int length, int style)
{
	byte *data;
	byte *cur;
	byte *cryptkey;

	int i, enclen, mult;

	mult = 1;
	switch(style)
	{
		case 4:
			cryptkey = keys_09;
			break;
		case 3:
			cryptkey = keys_10;
			break;
		case 2:
			cryptkey = keys_11;
			break;
		case 1:
		case 0:
			cryptkey = keys_11;
			mult = 0;
			break;
	}

	enclen = 8 + CIELMOD8(length);
	data = malloc(enclen);

	memset(data, 0, enclen); 
	memcpy(data+8, inc, length);


	for(i = 8; i <= enclen-8; i+=8)
	{
		cur = data+i;

		block_xor(cur, cur-8);

		if(mult)
		{
			block_xor(cur, cryptkey+16);
			block_obfus(cur);
			block_xor(cur, cryptkey+8);
		}

		block_swap(cur);
		block_obfus(cur);
		block_xor(cur, cryptkey);
        }

	memcpy(data, ident[style], 4);
	enclen -= 8;
	memcpy(data+4, &enclen, 4);
	return data;
}

int main(int argc, const char **argv)
{
	FILE *in, *out;
	int len;
	int style = 99;
	byte *decrypted, *encrypted;
	byte *cryptkey;

	printf("encrypt2dx by ryuuou\n\tencrypts unencrypted .2dx files (ORLY)\n\n");

	if(argc < 2)
	{
		printf("USAGE: encrypt2dx <file>\n\npress enter to exit\n");
		getchar();
		return 0;
	}

	in = fopen(argv[1], "rb");
		
	if(in == NULL)
	{
		printf("file not found: %s\n\npress enter to exit\n", argv[1]);
		getchar();
		return 0;
	}

	do
	{
		printf("select encryption (0-4)\n\t0: Troopers\n\t1: DistorteD, Happy Sky, GOLD\n\t2: RED\n\t3: 10th style\n\t4: 9th style\n");
		scanf("%d", &style);
	}
	while(!(style >= 0 && style <= 4));

	fseek(in, 0, SEEK_END);
	len = (int)ftell(in);
	fseek(in, 0, SEEK_SET);
		
	decrypted = malloc(len);

	fread(decrypted, len, 1, in);
		
	fclose(in);

	printf("encrypting...\n");
	encrypted = encrypt(decrypted, len, style);

	printf("writing encrypted.2dx to disk...\n");
	out = fopen("encrypted.2dx", "wb");
	fwrite(encrypted, 8+CIELMOD8(len), 1, out);
	fclose(out);

	free(decrypted);
	free(encrypted);
	
	printf("press enter to exit\n");
	getchar(); getchar();
	return 0;
}
