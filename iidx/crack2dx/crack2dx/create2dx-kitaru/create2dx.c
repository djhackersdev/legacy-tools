//create2dx.c by Kitaru
//Packs numerically sequenced .wav in folder ./input into an unencrypted .2dx.
//Based on crack2dx.c by Tau and a few chunks from create2dx.c by icex2.
//TODO: add different numbering specifications (BMS, BME, etc.)

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef unsigned char byte;

const byte keys_09[3][8] = {
    { 0x97, 0x1E, 0x24, 0xA0, 0x9A, 0x00, 0x10, 0x2B },
    { 0x91, 0xEF, 0xD7, 0x7A, 0xCD, 0x11, 0xAF, 0xAF },
    { 0x8D, 0x26, 0x5D, 0xBB, 0xE0, 0xC6, 0x1B, 0x2B }
};

const byte keys_10[3][8] = {
    { 0x2D, 0x86, 0x56, 0x62, 0xD7, 0xFD, 0xCA, 0xA4 },
    { 0xB3, 0x24, 0x60, 0x26, 0x24, 0x81, 0xDB, 0xC2 },
    { 0x57, 0xB1, 0x74, 0x6F, 0xA7, 0x52, 0x99, 0x21 }
};

const byte keys_11[3][8] = {
    { 0xED, 0xF0, 0x9C, 0x90, 0x44, 0x1A, 0x5A, 0x03 },
    { 0xAB, 0x07, 0xC1, 0x99, 0x23, 0x24, 0x32, 0xC7 },
    { 0x5F, 0x32, 0xA5, 0x97, 0xAD, 0x98, 0x0F, 0x8F }
};

const unsigned char aftertoc[24] = {
    '2', 'D', 'X', '9', 0x18, 0x00, 0x00, 0x00, 0x5A, 0x07, 0x01, 0x00, 0x30, 0x32, 0xFF, 0xFF, 
    0x40, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00
};

byte block[8];
byte *encrypted, *encpos;

void die(const byte *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    exit(EXIT_FAILURE);
}
//I guess we could encrypt it eventually, but I found DistorteD doesn't bother to reject non-encrypted .2dx, so I just commented everything out.
/*void encrypt_common()    //I guess I need to reverse the operands?
{
    byte a = block[0] * 63;
    byte b = a + block[3];
    byte c = block[1] * 17;
    byte d = c + block[2];
    byte e = d + b;
    byte f = e * block[3];
    byte g = f + b + 51;
    byte h = b ^ d;
    byte i = g ^ e;

    block[4] ^= h;
    block[5] ^= d;
    block[6] ^= i;
    block[7] ^= g;
}

void encrypt_normal(const byte *key1, const byte *key2, const byte *key3)
{
    byte tmp;
    int i;

	for (i = 0 ; i < 8 ; i++) block[i] ^= key3[i];
	encrypt_common();
	for (i = 0 ; i < 8 ; i++) block[i] ^= key2[i];

    for (i = 0 ; i < 4 ; i++) {
        tmp = block[i];
        block[i] = block[i + 4];
        block[i + 4] = tmp;
    }

	encrypt_common();
    for (i = 0 ; i < 8 ; i++) block[i] ^= key1[i];
}

void encrypt_09()
{
    encrypt_normal(keys_09[0], keys_09[1], keys_09[2]);
}

void encrypt_10()
{
    encrypt_normal(keys_10[0], keys_10[1], keys_10[2]);
}

void encrypt_11()
{
    encrypt_normal(keys_11[0], keys_11[1], keys_11[2]);
}

void encrypt_12()
{
    byte tmp;
    int i;

    for (i = 0 ; i < 4 ; i++) {
        tmp = block[i];
        block[i] = block[i + 4];
        block[i + 4] = tmp;
    }
	encrypt_common();

    for (i = 0 ; i < 8 ; i++) block[i] ^= keys_11[0][i];   
}

void encrypt(FILE *in, const char *filename)
{
    void (*encrypt_block)();
    byte curBlock[8], prevBlock[8];
    int i;

    memset(curBlock, 0, 8);
    memset(prevBlock, 0, 8);
	
	encrypt_block = encrypt_12; //eventually you'll be presented with a choice somehow

    while (!feof(in)) {
        fread(block, 8, 1, in);
        memcpy(curBlock, block, 8);

        encrypt_block();
        for (i = 0 ; i < 8 ; i++) {
            block[i] ^= prevBlock[i];
        }

        memcpy(encpos, block, 8);
        memcpy(prevBlock, curBlock, 8);

        encpos += 8;
    }
}*/

int main(int argc, byte **argv)
{
    const char *filename = argv[1];
    long length, length2;
    FILE *in;
	FILE *out;
	byte *buffer;
	char path1[511], path2[511];
	int *toc;

	if (argc != 2) die("Usage: %0 [.2dx input file]", argv[0]);

	int i = 0, ii = 0, wav = 0;
	char wavc[16];

	strcpy(path1, argv[0]);
	_strrev(path1);
	while(path1[i] != '\\')
		i++;
    while(path1[ii] != '\0')
		ii++;
    _strrev(path1);
	path1[ii - i] = '\0';
	strcat(path1,"input\\");

	while(1){
		wav++;
		printf("%d\n",wav);
		//getch();
		strcpy(path2,path1);
		sprintf(wavc,"%d.wav",wav);
		strcat(path2,wavc);

		in = fopen(path2, "rb");
		if (in == NULL){
			printf("Error opening %s\n", path2);
			wav--;
			break;}
		else{fclose(in);
			printf("       OK! %d\n",wav);}}
	
	in = NULL;

	toc = malloc(wav*sizeof(int));
	length = 0x48 + ((wav) * 4);

	for(i = 0; i < wav; i++){
		strcpy(path2,path1);
		sprintf(wavc,"%d.wav",i+1);
		strcat(path2,wavc);

		in = fopen(path2, "rb");

		if (in == NULL){
			printf("Error opening %s\n", path2);
			break;}
		else{
			printf("       OK! %d\n",i+1);
			fseek(in, 0, SEEK_END);
			toc[i] = length;
			printf("%d\n",toc[i]);
    		length += ftell(in) + 0x18;
			fclose(in);}}

	in = NULL;

    out = fopen(filename, "wb");

	fseek(out, 0x14, SEEK_SET);
	for(i = 0; i < 4; i++)
		fprintf(out,"%c",((wav>>(i*8))&255));
	

	fseek(out, 0x48, SEEK_SET);
	printf("%d",&toc[0]);
	for(i = 1; i < wav+1; i++)
		for(ii = 0; ii < 4; ii++){
			fprintf(out,"%c",(((toc[i-1])>>(ii*8))&255));}

	for (i = 0; i < wav; i++)
	{
		strcpy(path2,path1);
		sprintf(wavc,"%d.wav",i+1);
		strcat(path2,wavc);

		printf("File %s...", path2);

		in = fopen(path2, "rb");
	    fseek(in, 0, SEEK_END);
    	length2 = ftell(in);
	    fseek(in, 0, SEEK_SET);

		printf("reading %li...", length2);
		buffer = malloc(length2);
		fread(buffer, length2, 1, in);
		fclose(in);
		
		fseek(out, 0, SEEK_END);
		printf("writing...");
	    out = fopen(filename, "ab");
	    fwrite(aftertoc, 0x18, 1, out);
		fwrite(buffer, length2, 1, out);
	    fclose(out);
		printf("done\n");
	}

	encrypted = malloc(length);

	printf("Encrypting ...\n");
	out = fopen(filename, "rb");

	fread(block, 8, 1, in);
    //encpos = encrypted;
    //encrypt(out, filename);

	//out = fopen(filename, "wb");
	//fprintf(out,"%%e12");
	//fseek(out, 0x04, SEEK_SET);
	//for(i = 0; i < 4; i++)
		//fprintf(out,"%c",((length>>(i*8))&255));

	//fseek(out, 0x08, SEEK_SET);
	//fwrite(encrypted, length, 1, out);

    fclose(out);
    //split_files();

    free(encrypted);
    return EXIT_SUCCESS;
}
