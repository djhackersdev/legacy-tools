/****************************************************************************
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software
*    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned long getOffset(unsigned long songID, char* filename);
void loadData(FILE* infile, unsigned int data[33554432], char* filename);
void printUsage();
int getSlotAvailable(unsigned int stepMode, unsigned int songID);

short assignedTable[65536];

int main(int argc, char* argv[])
{
    FILE* infile;
    FILE* outfile;
    int numberOfEdits = 0;

    unsigned int *data;
    data=(unsigned int *)malloc(33554432*sizeof(unsigned int));

    for(int a = 0; a < 65536; a++)
        assignedTable[a] = 0;

    if((argc == 1) ||
      (strcmp(argv[1],"/help") == 0) ||
      (strcmp(argv[1],"/?") == 0) ||
      (strcmp(argv[1],"--help") == 0) ||
      (strcmp(argv[1],"--?") == 0))
    {
        printUsage();
        return 0;
    }

    data[33554431] = '!';

    numberOfEdits = argc - 3;

    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");

    if(infile == NULL)
    {
        printf("Error opening file \"%s\" - File does not exist or is in use by another program.", argv[1]);
        return 1;
    }

    if(outfile == NULL)
    {
        printf("Couldn't create file \"%s\" - Check for disk space and appropriate permissions.", argv[2]);
        return 2;
    }

    for(int b = 0; b < 33554432; b++)
        data[b] = fgetc(infile);

    fclose(infile);

    for(int c = 0; c < numberOfEdits; c++)
    {
        infile = fopen(argv[c+3], "r");

        if(infile == NULL)
        {
            char* tempFilename = argv[c+3];
            strcat(tempFilename, ".mcs");

            infile = fopen(argv[c+3], "r");

            if(infile == NULL)
                printf("Error opening edit file: \"%s\" - File does not exist or is in use by another program. Skipping...", argv[c+3]);           
            else
                loadData(infile, data, argv[c+3]);
        }
        else
            loadData(infile, data, argv[c+3]);

        fclose(infile);
    }

    for(int zz = 0; zz < 33554432; zz++)
        fprintf(outfile, "%c", data[zz]);

    fclose(outfile);
    free(data);
    return 0;
}

unsigned long getOffset(unsigned long songID, char* filename)
{
    if(songID == 0x00D4)
        return 0x01C00000; //MIDNITE BLAZE
    if(songID == 0x00D5)
        return 0x01C04000; //ORION.78 CIVILIZATION MIX
    if(songID == 0x00D6)
        return 0x01C08000; //SHARE MY LOVE
    if(songID == 0x00D3)
        return 0x01C0C000; //GROOVE
    if(songID == 0x012A)
        return 0x01C10000; //GROOVE 2K1
    if(songID == 0x0129)
        return 0x01C14000; //LET THE BEAT HIT 'EM CLASSIC R&B STYLE
    if(songID == 0x012D)
        return 0x01C18000; //DO IT RIGHT
    if(songID == 0x012C)
        return 0x01C1C000; //LOOK TO THE SKY
    if(songID == 0x012E)
        return 0x01C20000; //ON THE JAZZ
    if(songID == 0x012B)
        return 0x01C24000; //HEALING VISION ANGELIC MIX
    if(songID == 0x012F)
        return 0x01C28000; //DIVE - MORE DEEP AND DEEPER
    if(songID == 0x0130)
        return 0x01C2C000; //NORI NORI NORI
    if(songID == 0x0131)
        return 0x01C30000; //CENTRE OF THE HEART
    if(songID == 0x0132)
        return 0x01C34000; //LOVIN' YOU
    if(songID == 0x0133)
        return 0x01C38000; //ORDINARY WORLD
    if(songID == 0x0135)
        return 0x01C3C000; //SOMEWHERE OVER THE RAINBOW
    if(songID == 0x0136)
        return 0x01C40000; //FANTASY (DDRMAX)
    if(songID == 0x0137)
        return 0x01C44000; //WITCH DOCTOR
    if(songID == 0x0138)
        return 0x01C48000; //DO YOU REMEMBER ME
    if(songID == 0x013A)
        return 0x01C4C000; //HIGHS OFF U
    if(songID == 0x013B)
        return 0x01C50000; //MIRACLE
    if(songID == 0x013C)
        return 0x01C54000; //TWILIGHT ZONE
    if(songID == 0x013D)
        return 0x01C58000; //COWGIRL
    if(songID == 0x0140)
        return 0x01C5C000; //I'M IN THE MOOD FOR DANCING
    if(songID == 0x0141)
        return 0x01C60000; //LET'S GROOVE
    if(songID == 0x0144)
        return 0x01C64000; //WWW.BLONDE GIRL
    if(songID == 0x0145)
        return 0x01C68000; //MAX300 (GUESS)
    if(songID == 0x0146)
        return 0x01C6C000; //TRUE...
    if(songID == 0x0147)
        return 0x01C70000; //MY SWEET DARLIN'
    if(songID == 0x0148)
        return 0x01C74000; //SOBAKASU FRECKLES
    if(songID == 0x0149)
        return 0x01C78000; //SO DEEP
    if(songID == 0x014A)
        return 0x01C7C000; //FIREFLY
    if(songID == 0x014B)
        return 0x01C80000; //YOZORA NO MUKO
    if(songID == 0x014C)
        return 0x01C84000; //EXOTIC ETHNIC
    if(songID == 0x014D)
        return 0x01C88000; //CANDY STAR
    if(songID == 0x014E)
        return 0x01C8C000; //TRUE... TRANCE SUNRISE
    if(songID == 0x00EC)
        return 0x01C90000; //ASBOLUTE
    if(songID == 0x0101)
        return 0x01C94000; //ABYSS
    if(songID == 0x007C)
        return 0x01C98000; //AFRONOVA
    if(songID == 0x00BC)
        return 0x01C9C000; //LOVE AGAIN TONIGHT
    if(songID == 0x00BB)
        return 0x01CA0000; //B4U
    if(songID == 0x000B)
        return 0x01CA4000; //BRILLIANT 2U
    if(songID == 0x000D)
        return 0x01CA8000; //BRILLIANT 2U ORCHESTRAL GROOVE
    if(songID == 0x00ED)
        return 0x01CAC000; //BROKEN MY HEART
    if(songID == 0x00B8)
        return 0x01CB0000; //BURNING TEH FLOOR
    if(songID == 0x002A)
        return 0x01CB4000; //PARANOIA CLEAN
    if(songID == 0x00B7)
        return 0x01CB8000; //TRIP MACHINE CLIMAX
    if(songID == 0x007F)
        return 0x01CBC000; //DEAD END
    if(songID == 0x0083)
        return 0x01CC0000; //LUV TO ME AMD 2ND MIX
    if(songID == 0x009A)
        return 0x01CC4000; //DROP OUT
    if(songID == 0x00EE)
        return 0x01CC8000; //DXY!
    if(songID == 0x007E)
        return 0x01CCC000; //DYNAMITE RAVE
    if(songID == 0x00EB)
        return 0x01CD0000; //ECSTASY
    if(songID == 0x007D)
        return 0x01CD4000; //END OF THE CENTURY
    if(songID == 0x00C6)
        return 0x01CD8000; //ERA
    if(songID == 0x00E9)
        return 0x01CDC000; //HEALING VISION
    if(songID == 0x00B9)
        return 0x01CE0000; //HIGHER
    if(songID == 0x009F)
        return 0x01CE4000; //HYSTERIA
    if(songID == 0x010D)
        return 0x01CE8000; //INSERTION
    if(songID == 0x0080)
        return 0x01CEC000; //LA SENORITA
    if(songID == 0x010E)
        return 0x01CF0000; //CSFIL SPEED MIX
    if(songID == 0x002B)
        return 0x01CF4000; //TRIP MACHINE LUV MIX
    if(songID == 0x00FB)
        return 0x01CF8000; //MATSURI JAPAN
    if(songID == 0x00BA)
        return 0x01CFC000; //ORION.78 AMEURO MIX
    if(songID == 0x00DD)
        return 0x01D00000; //CELEBRATE NITE
    if(songID == 0x00C5)
        return 0x01D04000; //HOLIC
    if(songID == 0x001B)
        return 0x01D08000; //PARANOIA
    if(songID == 0x001D)
        return 0x01D0C000; //PARANOIA MAX DIRTY MIX
    if(songID == 0x00A0)
        return 0x01D10000; //PARANOIA EVOLUTION
    if(songID == 0x0082)
        return 0x01D14000; //PARANOIA REBIRTH
    if(songID == 0x0102)
        return 0x01D18000; //SANA MOLETTE NE ENTE
    if(songID == 0x00DE)
        return 0x01D1C000; //SEXY PLANET
    if(songID == 0x0081)
        return 0x01D20000; //SILENT HILL
    if(songID == 0x0019)
        return 0x01D24000; //AM-3P
    if(songID == 0x00EA)
        return 0x01D28000; //STILL IN MY HEART
    if(songID == 0x009B)
        return 0x01D2C000; //CSFIL
    if(songID == 0x00BE)
        return 0x01D30000; //MY SUMMER LOVE
    if(songID == 0x00C4)
        return 0x01D34000; //.59
    if(songID == 0x008B)
        return 0x01D38000; //DROP THE BOMB
    if(songID == 0x009E)
        return 0x01D3C000; //SUPER STAR
    if(songID == 0x009C)
        return 0x01D40000; //WILD RUSH
    if(songID == 0x0151)
        return 0x01D44000; //LONG TRAIN RUNNIN'
    if(songID == 0x0152)
        return 0x01D48000; //MAXIMUM OVERDRIVE
    if(songID == 0x0153)
        return 0x01D4C000; //THERE YOU'LL BE (GUESS)
    if(songID == 0x0154)
        return 0x01D50000; //WAKA LAKA
    if(songID == 0x0156)
        return 0x01D54000; //D2R (GUESS)
    if(songID == 0x0157)
        return 0x01D58000; //DESTINY (GUESS)
    if(songID == 0x0158)
        return 0x01D5C000; //LIVING IN AMERICA
    if(songID == 0x015A)
        return 0x01D60000; //SWEET SWEET LOVE MAGIC
    if(songID == 0x015B)
        return 0x01D64000; //EVER SNOW (GUESS)
    if(songID == 0x015D)
        return 0x01D68000; //THE REFLEX
    if(songID == 0x0160)
        return 0x01D6C000; //IT'S RAINING MEN
    if(songID == 0x0162)
        return 0x01D70000; //SECRET RENDEZ-VOUS
    if(songID == 0x0163)
        return 0x01D74000; //LITTLE BOY
    if(songID == 0x0164)
        return 0x01D78000; //RAIN OF SORROW
    if(songID == 0x0165)
        return 0x01D7C000; //MAXX UNLIMITED
    if(songID == 0x0166)
        return 0x01D80000; //DIVE TO THE NIGHT (GUESS)
    if(songID == 0x0167)
        return 0x01D84000; //TSUGARU
    if(songID == 0x0168)
        return 0x01D88000; //BREAKDOWN
    if(songID == 0x0169)
        return 0x01D8C000; //BURNING HEAT!
    if(songID == 0x016B)
        return 0x01D90000; //FANTASY (MAX2) (GUESS)
    if(songID == 0x016C)
        return 0x01D94000; //I FEEL...
    if(songID == 0x016D)
        return 0x01D98000; //CANDY STAR
    if(songID == 0x016E)
        return 0x01D9C000; //SPIN THE DISC
    if(songID == 0x0173)
        return 0x01DA0000; //KAKUMEI
    if(songID == 0x0174)
        return 0x01DA4000; //AFRONOVA FROM NONSTOP MEGAMIX (GUESS)
    if(songID == 0x0175)
        return 0x01DA8000; //AM-3P AM EAST MIX
    if(songID == 0x0176)
        return 0x01DAC000; //BRILLIANT 2U KOG G3 MIX
    if(songID == 0x0177)
        return 0x01DB0000; //B4U B4 ZA BEAT MIX
    if(songID == 0x0178)
        return 0x01DB4000; //DROP OUT KOG MIX (GUESS)
    if(songID == 0x0179)
        return 0x01DB8000; //DYNAMITE RAVE B4 ZA BEAT MIX (GUESS)
    if(songID == 0x017A)
        return 0x01DBC000; //HYSTERIA 2001
    if(songID == 0x017B)
        return 0x01DC0000; //MATSURI JAPAN NONSTOP MEGAMIX
    if(songID == 0x017C)
        return 0x01DC4000; //SEXY PLANET NONSTOP MEGAMIX
    if(songID == 0x017D)
        return 0x01DC8000; //SUPER STAR NONSTOP MEGAMIX
    if(songID == 0x017E)
        return 0x01DCC000; //STILL IN MY HEART MOMO MIX
    if(songID == 0x017F)
        return 0x01DD0000; //WILD RUSH NONSTOP MEGAMIX
    if(songID == 0x0180)
        return 0x01DD4000; //BURNIN' THE FLOOR BLUE FIRE MIX
    if(songID == 0x0181)
        return 0x01DD8000; //TSUGARU APPLE MIX
    if(songID == 0x0182)
        return 0x01DDC000; //ECSTASY MIDNIGHT BLUE MIX (GUESS)
    if(songID == 0x0183)
        return 0x01DE0000; //SILENT HILL 3RD CHRISTMAS MIX
    if(songID == 0x0184)
        return 0x01DE4000; //CELEBRATE NITE EURO TRANCE MIX
    if(songID == 0x0185)
        return 0x01DE8000; //HIGHER NEXT MORNING MIX (GUESS)
    if(songID == 0x0186)
        return 0x01DEC000; //MY SUMMER LOVE (TOMMY'S SMILE MIX)
    if(songID == 0x001A)
        return 0x01DF0000; //TRIP MACHINE
    if(songID == 0x001C)
        return 0x01DF4000; //SP-TRIP MACHINE JUNGLE MIX
    if(songID == 0x002E)
        return 0x01DF8000; //KEEP ON MOVIN'
    if(songID == 0x008A)
        return 0x01DFC000; //CUTIE CHASER
    if(songID == 0x00FC)
        return 0x01E00000; //REMEMBER YOU
    if(songID == 0x010B)
        return 0x01E04000; //AFRONOVA PRIMEVAL
    if(songID == 0x00B6)
        return 0x01E08000; //BABY BABY GIMME YOUR LOVE
    if(songID == 0x0085)
        return 0x01E0C000; //GRADIUSIC CYBER (AMD G5 MIX)
    if(songID == 0x008C)
        return 0x01E10000; //LA SENORITA VIRTUAL
    if(songID == 0x00C3)
        return 0x01E14000; //LEADING CYBER
    if(songID == 0x00C9)
        return 0x01E18000; //DON'T STOP AMD 2ND MIX
    if(songID == 0x00EF)
        return 0x01E1C000; //MR. T (TAKE ME HIGHER)
    if(songID == 0x00F1)
        return 0x01E20000; //I WAS THE ONE
    if(songID == 0x00F3)
        return 0x01E24000; //ELECTRO TUNED
    if(songID == 0x0086)
        return 0x01E28000; //GENTLE STRESS (AMD SEXUAL MIX)
    if(songID == 0x000E)
        return 0x01E2C000; //MAKE IT BETTER
    if(songID == 0x00F4)
        return 0x01E30000; //PARANOIA ETERNAL
    if(songID == 0x0089)
        return 0x01E34000; //AFTER THE GAME OF LOVE
    if(songID == 0x002C)
        return 0x01E38000; //LOVE THIS FEELING
    if(songID == 0x000A)
        return 0x01E3C000; //PUT YOUR FAITH IN ME
    if(songID == 0x0084)
        return 0x01E40000; //JAM JAM REGGAE (AMD SWING MIX)
    if(songID == 0x002D)
        return 0x01E44000; //THINK YA BETTER D
    if(songID == 0x0076)
        return 0x01E48000; //DAM DARIRAM
    if(songID == 0x00E1)
        return 0x01E4C000; //SYNCHRONIZED LOVE
    if(songID == 0x0121)
        return 0x01E50000; //SKY HIGH
    if(songID == 0x0007)
        return 0x01E54000; //LET'S GET DOWN
    if(songID == 0x0018)
        return 0x01E58000; //I BELIEVE IN MIRACLES
    if(songID == 0x00E5)
        return 0x01E5C000; //RHYTHM AND POLICE
    if(songID == 0x0017)
        return 0x01E60000; //IF YOU WERE HERE
    if(songID == 0x0016)
        return 0x01E64000; //GET UP'N MOVE
    if(songID == 0x0009)
        return 0x01E68000; //BUTTERFLY
    if(songID == 0x0023)
        return 0x01E6C000; //EL RITMO TROPICAL
    if(songID == 0x0078)
        return 0x01E70000; //CAPTAIN JACK
    if(songID == 0x0013)
        return 0x01E74000; //LITTLE BITCH
    if(songID == 0x00B4)
        return 0x01E78000; //SAINT GOES MARCHING
    if(songID == 0x013E)
        return 0x01E7C000; //TELEPHONE OPERATOR
    if(songID == 0x0197)
        return 0x01E80000; //MEMORIES
    if(songID == 0x0198)
        return 0x01E84000; //CRASH! (GUESS)
    if(songID == 0x019B)
        return 0x01E88000; //VANITY ANGEL
    if(songID == 0x0054)
        return 0x01E8C000; //GENOM SCREAMS
    if(songID == 0x0039)
        return 0x01E90000; //BE IN MY PARADISE
    if(songID == 0x0046)
        return 0x01E94000; //SKA A GO GO
    if(songID == 0x003E)
        return 0x01E98000; //DR. LOVE
    if(songID == 0x004B)
        return 0x01E9C000; //LUV TO ME DISCO MIX
    if(songID == 0x0043)
        return 0x01EA0000; //R3
    if(songID == 0x0037)
        return 0x01EA4000; //5.1.1.
    if(songID == 0x0033)
        return 0x01EA8000; //JAM JAM REGGAE
    if(songID == 0x003A)
        return 0x01EAC000; //E-MOTION
    if(songID == 0x0049)
        return 0x01EB0000; //CELEBRATE
    if(songID == 0x003D)
        return 0x01EB4000; //20, NOVEMBER
    if(songID == 0x01A5)
        return 0x01EB8000; //DROP THE BOMB (SYSTEM SF MIX)
    if(songID == 0x014F)
        return 0x01EBC000; //KIND LADY
    if(songID == 0x0150)
        return 0x01EC0000; //SO IN LOVE
    if(songID == 0x008D)
        return 0x01EC4000; //AM-3P 303 BASS MIX
    if(songID == 0x01AC)
        return 0x01EC8000; //CUTIE CHASER (MORNING MIX)
    if(songID == 0x01A6)
        return 0x01ECC000; //DYNAMITE RAVE (DOWN BIRD SOTA MIX)
    if(songID == 0x019D)
        return 0x01ED0000; //DO IT RIGHT (HARMONIZED 2STEP MIX)
    if(songID == 0x01A0)
        return 0x01ED4000; //LOOK TO THE SKY TRUE COLOR MIX
    if(songID == 0x01B0)
        return 0x01ED8000; //WE WILL ROCK YOU
    if(songID == 0x01B1)
        return 0x01EDC000; //IRRESISTIBLEMENT
    if(songID == 0x01B3)
        return 0x01EE0000; //SPEED OVER BEETHOVEN
    if(songID == 0x01C2)
        return 0x01EE4000; //WE ARE THE CHAMPIONS
    if(songID == 0x01AF)
        return 0x01EE8000; //LA COPA DE LA VIDA
    if(songID == 0x01B2)
        return 0x01EEC000; //CARTOON HEROES
    if(songID == 0x01C3)
        return 0x01EF0000; //I DO I DO I DO
    if(songID == 0x01E3)
        return 0x01EF4000; //BURNING THE FLOOR MOMO MIX
    if(songID == 0x01E5)
        return 0x01EF8000; //SENORITA SPEEDY MIX
    if(songID == 0x01B4)
        return 0x01EFC000; //KISS KISS KISS
    if(songID == 0x01B5)
        return 0x01F00000; //LOVE LOVE SUGAR
    if(songID == 0x01B6)
        return 0x01F04000; //L'AMOUR ET LA LIBERTE
    if(songID == 0x01B8)
        return 0x01F08000; //DESTINY LOVERS
    if(songID == 0x01B9)
        return 0x01F0C000; //THE LEAST 100 SECONDS
    if(songID == 0x01BA)
        return 0x01F10000; //GAMELAN DE COUPLE
    if(songID == 0x01BC)
        return 0x01F14000; //MOBO*MOGA
    if(songID == 0x01BD)
        return 0x01F18000; //BE LOVIN
    if(songID == 0x01BE)
        return 0x01F1C000; //MIRACLE MOON
    if(songID == 0x01C0)
        return 0x01F20000; //V
    if(songID == 0x01C1)
        return 0x01F24000; //A
    if(songID == 0x01C4)
        return 0x01F28000; //COLORS
    if(songID == 0x01C7)
        return 0x01F2C000; //WHITE LOVERS
    if(songID == 0x01C8)
        return 0x01F30000; //FROZEN RAY
    if(songID == 0x01CA)
        return 0x01F34000; //FEELING OF LOVE
    if(songID == 0x01CB)
        return 0x01F38000; //DAIKENKAI
    if(songID == 0x01CC)
        return 0x01F3C000; //DOOR OF MAGIC
    if(songID == 0x01CD)
        return 0x01F40000; //HOLD ON ME
    if(songID == 0x01CE)
        return 0x01F44000; //JET WORLD
    if(songID == 0x01C6)
        return 0x01F48000; //KISS ME ALL NIGHT LONG
    if(songID == 0x01CF)
        return 0x01F4C000; //HAPPY WEDDING
    if(songID == 0x01B7)
        return 0x01F50000; //I'M GONNA GET YOU
    if(songID == 0x01D5)
        return 0x01F54000; //SYNC
    if(songID == 0x01D6)
        return 0x01F58000; //STOIC
    if(songID == 0x01D7)
        return 0x01F5C000; //321 STARS
    if(songID == 0x01C5)
        return 0x01F60000; //STAY
    if(songID == 0x01BB)
        return 0x01F64000; //JANEJANA
    if(songID == 0x01D4)
        return 0x01F68000; //MIKENEKO ROCK
    if(songID == 0x01DB)
        return 0x01F6C000; //SAKURA
    if(songID == 0x01DD)
        return 0x01F70000; //HEAVEN IS A '57 METALLIC GREY
    if(songID == 0x01D9)
        return 0x01F74000; //TWINBEE GENERATION X
    if(songID == 0x01DA)
        return 0x01F78000; //XENON
    if(songID == 0x01C9)
        return 0x01F7C000; //LAST MESSAGE
    if(songID == 0x01DC)
        return 0x01F80000; //PINK ROSE
    if(songID == 0x01DF)
        return 0x01F84000; //LA BAMBA
    if(songID == 0x01E2)
        return 0x01F88000; //AIR
    if(songID == 0x01E6)
        return 0x01F8C000; //ACROSS THE NIGHTMARE
    if(songID == 0x01D1)
        return 0x01F90000; //BAG
    if(songID == 0x01D2)
        return 0x01F94000; //THE LEGEND OF MAX
    if(songID == 0x01BF)
        return 0x01F98000; //GRADUATION
    if(songID == 0x01AD)
        return 0x01F9C000; //LOVE SHINE
    if(songID == 0x01AE)
        return 0x01FA0000; //PARANOIA SURVIVOR
    if(songID == 0x01D0)
        return 0x01FA4000; //PARANOIA SURVIVOR MAX
    if(songID == 0x01D3)
        return 0x01FA8000; //AOI SHOUDOU
    if(songID == 0x01D8)
        return 0x01FAC000; //1998
    if(songID == 0x01DE)
        return 0x01FB0000; //HYPER EUROBEAT
    if(songID == 0x01E0)
        return 0x01FB4000; //DANCE DANCE REVOLUTION
    if(songID == 0x01E1)
        return 0x01FB8000; //TRIP MACHINE SURVIVOR
    if(songID == 0x01E4)
        return 0x01FBC000; //TEARS
/*    if(songID == 0x0000)
        return 0x01FC0000; //
    if(songID == 0x0000)
        return 0x01FC4000; //
    if(songID == 0x0000)
        return 0x01FC8000; //
    if(songID == 0x0000)
        return 0x01FCC000; //
    if(songID == 0x0000)
        return 0x01FD0000; //
    if(songID == 0x0000)
        return 0x01FD4000; //
    if(songID == 0x0000)
        return 0x01FD8000; //
    if(songID == 0x0000)
        return 0x01FDC000; //
    if(songID == 0x0000)
        return 0x01FE0000; //
    if(songID == 0x0000)
        return 0x01FE4000; //
    if(songID == 0x0000)
        return 0x01FE8000; //
    if(songID == 0x0000)
        return 0x01FEC000; //
    if(songID == 0x0000)
        return 0x01FF0000; //
    if(songID == 0x0000)
        return 0x01FF4000; //
    if(songID == 0x0000)
        return 0x01FF8000; //
    if(songID == 0x0000)
        return 0x01FFC000; //*/
    printf("Error: Unrecognized songID in file \"%s\": \"%X\"\n", filename, songID);
    return 0x01FFC000; //This slot should be unused.
}

void loadData(FILE* infile, unsigned int data[33554432], char* filename)
{
    bool extendedFormat = false;
    long tempIndex = 0;
    long tempIndex2 = 0;
    unsigned long offset = 0;
    int slotOffset = 0; //This number determines the offset from 'offset' where the data goes
                        //based on single/double and which edit slot(s) are used;
    unsigned int inputData[8192];
    unsigned int outputData[4096];
    unsigned long songID = 0;
    unsigned int stepMode = 0;
    unsigned int MDAT[2];
    unsigned int machineCode[2];
    unsigned int songIDbytes[2];
    unsigned int difficultyLevel;
    unsigned int difficultyFeet;
    unsigned int freezeFlag;
    for(int e = 0; e < 8192; e++)
        inputData[e] = fgetc(infile);
//    inputData[777]; //MSB of songID
//    inputData[776]; //LSB of songID

    songID = inputData[777] * 256;
    songID &= 0x0000FFFF;
    songID += inputData[776];

    stepMode = inputData[710];

    offset = getOffset(songID, filename);

    if(offset == 33538048)
        return;

    slotOffset = getSlotAvailable(stepMode, songID);
    
    if(slotOffset == -1)
    {
        if(stepMode == 0 || stepMode == 1)
        {
            printf("Unable to insert the edit data in file \"%s\" -\nReason: Too many singles/couples edits used already for that song. Skipping...\n", filename);
        }
        else if(stepMode == 2)
        {
            printf("Unable to insert the edit data in file \"%s\" -\nReason: Too many double edits used already for that song. Skipping...\n", filename);
        }
        else if(stepMode == 3)
        {
            printf("Unable to insert the edit data in file \"%s\" -\nReason: 6Panel edits are not supported on this version. Skipping...\n", filename);
        }
        else
        {
            printf("Unable to insert the edit data in file \"%s\" -\nReason: Invalid step type found. Skipping...\n", filename);
        }
        return;
    }


    MDAT[0] = inputData[704];
    MDAT[1] = inputData[705];

    machineCode[0] = inputData[688];
    machineCode[1] = inputData[689];

    songIDbytes[0] = inputData[708];
    songIDbytes[1] = inputData[709];

    stepMode = inputData[710];

    difficultyLevel = inputData[711];
    difficultyFeet = inputData[778];

    freezeFlag = inputData[779];

    if(freezeFlag == 3)
        extendedFormat = true;

/* Begin 4-byte intro */
    outputData[0] = '\xAD';
    outputData[1] = '\xDE';
    outputData[2] = '\xCE';
    outputData[3] = '\xFA';
/* End 4-byte intro */

/* Begin title strip */
    outputData[4] = 0x5B;

    if(inputData[178] == 130) //If the name is written in Unicode instead, for some reason...
    {
        outputData[5] = inputData[22];
        outputData[6] = inputData[23];
        outputData[7] = inputData[24];
        outputData[8] = inputData[25];
        outputData[9] = inputData[26];
        outputData[10] = inputData[27];
        outputData[11] = inputData[28];
        outputData[12] = inputData[29];
    }
    else
    {
        outputData[5] = inputData[178];
        outputData[6] = inputData[179];
        outputData[7] = inputData[180];
        outputData[8] = inputData[181];
        outputData[9] = inputData[182];
        outputData[10] = inputData[183];
        outputData[11] = inputData[184];
        outputData[12] = inputData[185];
    }
    outputData[13] = 0x5D;
/* End 4-byte intro */    

    outputData[14] = 0; // ???
    outputData[15] = 0; // ???
    outputData[16] = 0; // ???
    outputData[17] = 0; // ???
    outputData[18] = 0; // ???
    outputData[19] = stepMode;
    outputData[20] = 2; // ???
    outputData[21] = 2; // ???
    outputData[22] = 2; // ???
    outputData[23] = 17; // ???
    outputData[24] = 1; // ???
    outputData[25] = 0; // ???
    outputData[26] = 5; // ???
    outputData[27] = 0; // ???
    outputData[28] = 32; // ???
    outputData[29] = 0; // ???
    outputData[30] = 1; // ???
    outputData[31] = 0; // ???
    outputData[32] = 0; // ???
    outputData[33] = 0; // ???
    outputData[34] = 0; // ???
    outputData[35] = 0; // ???
    outputData[36] = 2; // ???
    outputData[37] = 0; // ???
    outputData[38] = 0; // ???
    outputData[39] = 0; // ???
    outputData[40] = 0; // ???
    outputData[41] = 0; // ???
    outputData[42] = 0; // ???
    outputData[43] = 0; // ???

//The bytes from 14 through 43 are currently unknown!

/* Beginning of --DanceDanceRevolution-- line */
    outputData[44] = '-';
    outputData[45] = '-';
    outputData[46] = 'D';
    outputData[47] = 'a';
    outputData[48] = 'n';
    outputData[49] = 'c';
    outputData[50] = 'e';
    outputData[51] = 'D';
    outputData[52] = 'a';
    outputData[53] = 'n';
    outputData[54] = 'c';
    outputData[55] = 'e';
    outputData[56] = 'R';
    outputData[57] = 'e';
    outputData[58] = 'v';
    outputData[59] = 'o';
    outputData[60] = 'l';
    outputData[61] = 'u';
    outputData[62] = 't';
    outputData[63] = 'i';
    outputData[64] = 'o';
    outputData[65] = 'n';
    outputData[66] = '-';
    outputData[67] = '-';
    outputData[68] = machineCode[0];
    outputData[69] = machineCode[1];
/* End of --DanceDanceRevolution--** line */

    for(int f = 0; f < 14; f++)
    {
        tempIndex = 70 + f;
        outputData[tempIndex] = 0;
    }

    if(extendedFormat)
    {
        outputData[84] = MDAT[0];
        outputData[85] = MDAT[1];
    }
    else
    {
        outputData[84] = MDAT[0];
        outputData[85] = MDAT[1] - 1;
    }
    outputData[86] = 0;
    outputData[87] = 0;
    outputData[88] = songIDbytes[0];
    outputData[89] = songIDbytes[1];

    outputData[90] = stepMode;
    outputData[91] = difficultyLevel;

    for(int g = 0; g < 56; g++)
    {
        tempIndex = 92 + g;
        outputData[tempIndex] = 0;
    }

    for(int h = 0; h < 16; h++)
    {
        tempIndex = 148 + h;
        tempIndex2 = 768 + h;
        outputData[tempIndex] = inputData[tempIndex2];
    }

    if(extendedFormat)
    {
        for(int y = 0; y < 3676; y++)
        {
            tempIndex =  164 + y;
            tempIndex2 = 1040 + y;
            outputData[tempIndex] = inputData[tempIndex2];
        }
    }
    else
    {
        for(int y = 0; y < 3932; y++)
        {
            tempIndex =  164 + y;
            tempIndex2 = 784 + y;
            outputData[tempIndex] = inputData[tempIndex2];
        }
    }

    for(int z = 0; z < 4096; z++)
    {
        tempIndex = offset + slotOffset + z;
        data[tempIndex] = outputData[z];
    }
    return;
}

void printUsage()
{
    printf("StepLoader.exe usage:\n");
    printf("  StepLoader (inputfile) (outputfile) edit1 [...]\n");
    printf("StepLoader uses .mcs-formatted files. Specifying the .mcs extension is not necessary.\n");
    return;
}

int getSlotAvailable(unsigned int stepMode, unsigned int songID)
{
    if(stepMode == 0 || stepMode == 1)
    {
        if((assignedTable[songID] & 1))
            if((assignedTable[songID] & 2))
                return -1;
            else
            {
                assignedTable[songID] |= 2;
                return 4096;
            }
        else
        {
            assignedTable[songID] |= 1;
            return 0;
        }
    }
    else if(stepMode == 2)
    {
        if((assignedTable[songID] & 4))
            if((assignedTable[songID] & 8))
                return -1;
            else
            {
                assignedTable[songID] |= 8;
                return 12288;
            }
        else
        {
            assignedTable[songID] |= 4;
            return 8192;
        }
    }
    else
        return true;
}
