//common.h


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


#define MAXWAVS (36*36-1) //wav files from 01-0A-0Z-10-ZZ (you get it) = 36*36 - 1
#define CHARTCOUNT 7 //beginner + (normal,hyper,another times two), what about kaiden in gold cs?

#define PNEWLINE printf("\n") //used too often

#define CIELMOD8(a) (!(a%8)?(a):((a)+(8-(a)%8))) //rounds (a) up to the next mod 8 int

typedef unsigned char byte;

//chartorder inside .1 file: h7, n7, a7, b, h14, n14, a14
const char *BMENAMES[] = { "h7.bme", "n7.bme", "a7.bme", "b.bme", "h14.bme", "n14.bme", "a14.bme" }; //we use constant filenames here for ease of coding/use
const char *CHARTNAMES[] = { "h7", "n7", "a7", "b", "h14", "n14", "a14" };
int style; //produce files for this iidx version; 0 == Troopers, 1 == GOLD, 2 == DistorteD and Happy Sky
float factor; //us this factor to strech .1 event time indices (based on version); 1 for Troopers, FPS/1000 for everything else


//helper functions for BME numbering weirdness;
//converts a number to a 01-ZZ style string and places that in the first two bytes of str
void itostr(char *str, int i)
{
	str[0] = i / 36; //works because this floors
	if(str[0] > 9)
		str[0] += 7; //there's a gap of 8 characters between '9' and 'A'
	str[0] += 48; //fancy charactermap magic

	str[1] = i % 36; //mod operator = <3
	if(str[1] > 9)
		str[1] += 7; 
	str[1] += 48;
}

//the above in inverse; convert a BME-style numbering to int.
//stupidly tries to convert the first two bytes of *str
int strtoi(char *str)
{
	int a,b,i;

	a = str[0] - 48;
	if(a > 9)
		a -= 7;
	
	b = str[1] - 48;
	if(b > 9)
		b -= 7;

	i = 36*a + b;

	return i;
}


//does the same as the above but converts from hex instead
int strtoi_h(char *str)
{
	int a,b,i;

	a = str[0] - 48;
	if(a > 9)
		a -= 7;
	
	b = str[1] - 48;
	if(b > 9)
		b -= 7;

	i = 16*a + b;

	return i;
}
