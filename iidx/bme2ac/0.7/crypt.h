
//crypt.h
//code for encrypting .2dx files with DistorteD's algorithm/key

//apparently konami's internal implementation of this doesn't do proper boundchecking and also doesn't zero its buffers, so there is some garbage left behind in the original files

//format of encrypted .2dx file:
//header:
//4 bytes: identifier ("%e12" in case of distorted)
//4 bytes: length of the decrypted .2dx file
//after this follows the encrypted data


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

byte cryptkey[8] = { 0xED, 0xF0, 0x9C, 0x90, 0x44, 0x1A, 0x5A, 0x03 }; //DistorteD'S encryption key

//XORs two 8 byte blocks
inline void block_xor(byte *data, byte *parm)
{
	int i;
	for (i = 0 ; i < 8 ; i++) 
		data[i] ^= parm[i];
	return;
}

//Swaps the lower 4 bytes with the upper 4 bytes
inline void block_swap(byte *data)
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
inline void block_obfus(byte *data)
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

//encrypts the .2dx file; this simply takes the reverse way from crack2dx.c by an unknown author (QQQ again, but HOLY SHIT that code is a mess (and buggy))
int encrypt_2dx(byte *dec, int len, byte **enc)
{
	byte *cur; //current block of 8 bytes
	int enclen;

	int i;

	//ciel to mod 8;
	
	enclen = len + 8; //length of the encoded 2dx = length of unencoded 2dx + headersize (8 bytes)
	
	if(enclen%8)
	  enclen += 8-enclen%8;

	*enc = malloc(enclen);
	memset(*enc, 0, enclen); 
	memcpy(*enc+8, dec, len); //fill up the encoded array with unencoded stuff

	printf("encrypting .2dx file...\n");
	for(i = 8; i <= enclen - 8; i+=8) //encode in blocks of 8 bytes, start at byte 8 of data
	{
		cur = *enc+i;

		//Step 1: XOR the current block with the encrypted previous block
		block_xor(cur, cur-8);

		//Step 2: Swap the lower and upper 4 bytes of the block
		block_swap(cur);

		//Step 3: Further obfuscation with XOR; the lower 4 bytes are untouched, so encrypting works the same as decrypting
		block_obfus(cur);

		//Step 4: XOR the block (yet again) with DistorteD's key
		block_xor(cur, keys_11);
        }

	memcpy(*enc, "%hid", 4); //encryption identifier (Troopers)

	memcpy(*enc+4, &len, 4); //write the length of the decoded 2dx file

	return enclen;
}

int decrypt_2dx(byte *enc, byte **dec)
{
	int i;
	int mult;
	int len;
	byte *buf;
	byte *cur, *prev;
	byte *cryptkey;

	buf = enc;

	//identify iidx version

	mult = 1;
	if(!strncmp(buf, "%eNc", 4))
	{
		cryptkey = keys_09; //9th style
	}
	else if(!strncmp(buf, "%e10", 4))
	{
		cryptkey = keys_10; //10th style
	}
	else if(!strncmp(buf, "%e11", 4))
	{
		cryptkey = keys_11; //RED
	}
	else if(!strncmp(buf, "%e12", 4))
	{
		cryptkey = keys_11; //HS+
		mult = 0;
	}
	else if(!strncmp(buf, "%hid", 4))
	{
		cryptkey = keys_11; //Troopers
		mult = 0;
	}
	else if(!strncmp(buf, "%iO0", 4))
	{
		cryptkey = keys_16; //Empress
	}
	else
	{
		return -1;
	}

	len = *((int*)(enc + 4));
	
	if(len%8)
	  len += 8-len%8;

	
	*dec = malloc(len);
	//decrypt

	if(!(mult == 2)) //skip for unencrypted
	{
		memcpy(*dec, buf+8, len-8); //copy the encrypted part
		memset(buf, 0, 8); //zero the first block to make the loop easier

		for(i = 0; i <= len-16; i+= 8)
		{
			cur = *dec + i; //cur points to the current encrypted block to decrypt
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

	return len;
}