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

byte block[8];
byte *decrypted, *decpos;

void die(const byte *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    exit(EXIT_FAILURE);
}

void decrypt_common()
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

void decrypt_normal(const byte *key1, const byte *key2, const byte *key3)
{
    byte tmp;
    int i;

    for (i = 0 ; i < 8 ; i++) block[i] ^= key1[i];
    decrypt_common();

    for (i = 0 ; i < 4 ; i++) {
        tmp = block[i];
        block[i] = block[i + 4];
        block[i + 4] = tmp;
    }

    for (i = 0 ; i < 8 ; i++) block[i] ^= key2[i];
    decrypt_common();

    for (i = 0 ; i < 8 ; i++) block[i] ^= key3[i];
}

void decrypt_09()
{
    decrypt_normal(keys_09[0], keys_09[1], keys_09[2]);
}

void decrypt_10()
{
    decrypt_normal(keys_10[0], keys_10[1], keys_10[2]);
}

void decrypt_11()
{
    decrypt_normal(keys_11[0], keys_11[1], keys_11[2]);
}

void decrypt_12()
{
    byte tmp;
    int i;

    for (i = 0 ; i < 8 ; i++) block[i] ^= keys_11[0][i];
    decrypt_common();

    for (i = 0 ; i < 4 ; i++) {
        tmp = block[i];
        block[i] = block[i + 4];
        block[i + 4] = tmp;
    }
}

void decrypt(FILE *in, const char *filename)
{
    void (*decrypt_block)();
    byte curBlock[8], prevBlock[8];
    int i;

    memset(curBlock, 0, 8);
    memset(prevBlock, 0, 8);

    switch (block[2])
    {
    case 'N':
        if (block[3] != 'c') die("%s: Unknown encryption type", filename);
        decrypt_block = decrypt_09;

        break;

    case '1':
        switch (block[3])
        {
        case '0':
            decrypt_block = decrypt_10;
            break;

        case '1':
            decrypt_block = decrypt_11;
            break;

        case '2':
            decrypt_block = decrypt_12;
            break;

        default:
            die("%s: Unknown encryption type", filename);
            break;
        }

        break;

    default:
        die("%s: Unknown encryption type", filename);
        break;
    }

    /* Decrypt loop */
    while (!feof(in)) {
        fread(block, 8, 1, in);
        memcpy(curBlock, block, 8);

        decrypt_block();
        for (i = 0 ; i < 8 ; i++) {
            block[i] ^= prevBlock[i];
        }

        memcpy(decpos, block, 8);
        memcpy(prevBlock, curBlock, 8);

        decpos += 8;
    }
}

void extract(int file_num, byte *data)
{
    char filename[16];
    int *length = (int *) (data + 4);
    FILE *out;

    sprintf(filename, "%d.wav", file_num);
    printf("Dumping %s ...\n", filename);
    
    out = fopen(filename, "wb");
    fwrite(data, *length + 8, 1, out);
    fclose(out);
}

void split_files()
{
    int *nfiles = (int *) (decrypted + 0x14);
    int *toc = (int *) (decrypted + 0x48);
    int i;

    for (i = 0 ; i < *nfiles ; i++) {
        extract(i + 1, decrypted + toc[i] + 0x18);
    }
}

int main(int argc, byte **argv)
{
    const char *filename = argv[1];
    long length;
    FILE *in;

    if (argc != 2) die("Usage: %0 [.2dx input file]", argv[0]);

    in = fopen(filename, "rb");
    if (in == NULL) die("Error opening %s", filename);

    fseek(in, 0, SEEK_END);
    length = ftell(in);
    fseek(in, 0, SEEK_SET);

    decrypted = malloc(length);

    fread(block, 8, 1, in);
    if (block[0] == '%' && block[1] == 'e') {
        printf("Decrypting ...\n");

        decpos = decrypted;
        decrypt(in, filename);
    } else {
        fseek(in, 0, SEEK_SET);
        fread(decrypted, length, 1, in);
    }

    fclose(in);
    split_files();

    free(decrypted);
    return EXIT_SUCCESS;
}