/* cs2ac by 697＠14 */
/*
	09/05/22 v1.0	CS GOLD専用版
	09/05/24 v1.1	CS DJTに仮対応
	09/05/28 v1.2	リファクタリング
	09/05/30 v1.3	CS DistorteD以前に仮対応
*/


/*	作業ながらのあれこれ
/*
	0x07: 3rd style CS BGM autoplay
	0x0A: ??? style CS BGM autoplay
	0x82, 0x82: 3rd style CS キー音変更?
	旧CSバージョンDP譜面終了宣言イベントバグ対応
	BGM自動演奏（index==1）が複数発動させない
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAGNIA 16.71309 // 3rd style - 8th style
#define MAGNIB 16.68335	// 9th style - DJ TROOPERS

#define NAMEMAX 32

double	version = 1.3;

FILE	*in, *out;
int	i_timeA;
double	i_timeB;
int	d_type;
int	b_data[3];
int	t_notes[7];	// 0:SPN 1:SPH 2:SPA 3:SPB 4:DPN 5:DPN 6:DPA
int	f_size[7];	// 0:SPN 1:SPH 2:SPA 3:SPB 4:DPN 5:DPN 6:DPA
int	head_adrs;
int	f_exist[7];	// 0:SPN 1:SPH 2:SPA 3:SPB 4:DPN 5:DPN 6:DPA
int	filetype;
int	t_base_type;
int	maxwav;
char	inName[NAMEMAX],
	outName[NAMEMAX];

/* ヘッダ書き換え */
int headWritingUnit(int dif){
	int	adrsA, adrsB;
	
	switch(dif) {
	    case 0: adrsA = 0x08; adrsB = 0x0c; break; // SPN
	    case 1: adrsA = 0x00; adrsB = 0x04; break; // SPH
	    case 2: adrsA = 0x10; adrsB = 0x14; break; // SPA
	    case 3: adrsA = 0x18; adrsB = 0x1c; break; // SPB
	    case 4: adrsA = 0x38; adrsB = 0x3c; break; // DPN
	    case 5: adrsA = 0x30; adrsB = 0x34; break; // DPH
	    case 6: adrsA = 0x40; adrsB = 0x44; break; // DPA
	}
	
	fseek(out, adrsA, SEEK_SET);
	fwrite(&head_adrs, 1, 4, out);
	fseek(out, adrsB, SEEK_SET);
	fwrite(&f_size[dif], 1, 4, out);
	head_adrs += f_size[dif];
	
	return 0;
}

/* データ書き込み */
int dataWritingUnit(int dif){
	i_timeB   = i_timeA;
	switch(t_base_type){
	    case 0:
		i_timeB  *= MAGNIA;
		break;
	    case 1:
		i_timeB  *= MAGNIB;
		break;
	}
	i_timeA   = i_timeB;
	fwrite(&i_timeA,   1, 4, out);
	fwrite(&d_type,    1, 1, out);
	fwrite(&b_data[0], 1, 1, out);
	fwrite(&b_data[1], 1, 1, out);
	fwrite(&b_data[2], 1, 1, out);
	f_size[dif] += 8;
	
	return 0;
}

int cs2ac(int dif){
  /* 変数設定 */
	int	i;
	int	temp;
	int	dp_flag;
	int	rtm_flag = 0;
	int	start_flag = 0;
	int	wav01_auto = 0;
	
  /* 処理部分 */
    /* 変数初期化 */
	t_base_type  = 0;
	f_size[dif]  = 0;
	t_notes[dif] = 0;
	
	if(dif >= 4){
		dp_flag = 1;
	}
	else {
		dp_flag = 0;
	}
	
	fseek(out, head_adrs, SEEK_SET);
	
    /* サブヘッダ書き込み */
      /* [00 00 00 00 10 00 00 00][00 00 00 00 10 01 00 00] */
	temp = 0x00;
	for(i=0;i<0x04;i++){
		fwrite(&temp, 1, 1, out);
	}
	temp = 0x10;
	fwrite(&temp, 1, 1, out);
	temp = 0x00;
	for(i=0;i<0x07;i++){
		fwrite(&temp, 1, 1, out);
	}
	temp = 0x0110;
	fwrite(&temp, 1, 2, out);
	temp = 0x00;
	for(i=0;i<0x02;i++){
		fwrite(&temp, 1, 1, out);
	}
	f_size[dif] += 16;
	
    /* 譜面終端まで繰り返し */
	while((d_type != 0x06)||((dp_flag != 2)&&(dp_flag != 5))) {
	    /* データ初期化 */
		i_timeA	  = 0x00;
		d_type	  = 0x00;
		b_data[0] = 0x00;
		b_data[1] = 0x00;
		b_data[2] = 0x00;
		
	    /* データ読み込み */
		switch(filetype){
		    case 1: case 2: // CS3(DJT,GOLD),CS(DistorteD - 7th style)
			fread(&i_timeA,   1, 4, in);
			fread(&d_type,    1, 1, in);
			fread(&b_data[0], 1, 1, in);
			fread(&b_data[1], 1, 1, in);
			fread(&b_data[2], 1, 1, in);
			break;
			
		    case 3: // CS2 (6th style - 3rd style)
			fread(&i_timeA,   1, 2, in);
			fread(&d_type,    1, 1, in);
			fread(&b_data[0], 1, 1, in);
			b_data[1] = 0x00;
			b_data[2] = 0x00;
			break;
		}

		//一部の旧CSバージョン譜面バグ対応（DPなのに2Pの譜面終了宣言イベントがない）
		if((short int)i_timeA == 0x7FFF && dp_flag == 3)
			d_type = 0x06;
					
	    /* データタイプで振り分け */
		switch(d_type) {
		  /* ノートイベント */
		    case 0x00: case 0x10: case 0x20: case 0x30: case 0x40: case 0x50: case 0x60: case 0x70:
		    case 0x01: case 0x11: case 0x21: case 0x31: case 0x41: case 0x51: case 0x61: case 0x71:
			if (start_flag == 0){ break; }
			t_notes[dif]++;
			
			b_data[0] = d_type / 0x10;
			d_type   %= 0x10;
			
			dataWritingUnit(dif);
			break;
			
		  /* キー音変更 */
		    case 0x02: case 0x12: case 0x22: case 0x32: case 0x42: case 0x52: case 0x62: case 0x72:
		    case 0x03: case 0x13: case 0x23: case 0x33: case 0x43: case 0x53: case 0x63: case 0x73:
			switch(filetype){
			    case 3: b_data[1] = b_data[0]; break;
			}
			if (maxwav < (b_data[2]*0x100 + b_data[1])){
				maxwav  = b_data[2]*0x100 + b_data[1];
			}
			if ((b_data[2]*0x100 + b_data[1]) == 0){
				printf("\nFile \"00\" assigned ! : %08x : %02x\n",i_timeA ,d_type);
			}
			
		  /* 小節線 */
		    case 0x0C: case 0x1C:
			b_data[0] = d_type / 0x10;
			d_type   %= 0x10;
			
			dataWritingUnit(dif);
			break;
			
		  /* 判定幅設定 */
		    case 0x08: case 0x18: case 0x28: case 0x38: case 0x48: case 0x58:
			switch(filetype){
			    case 3:	// 6th style 解析バグ対策
				if(d_type == 0x48){
					b_data[1] = 0x08;
					b_data[0] = 0x04;
				}
				else {
					b_data[1] = b_data[0];
					b_data[0] = d_type / 0x10;
				}
				break;
				
			    default:
				b_data[1] = b_data[0];
				b_data[0] = d_type / 0x10;
				break;
			}
			d_type   %= 0x10;
			
			dataWritingUnit(dif);
			break;
			
		  /* 自動演奏 */
		    case 0xF7:
			break;
		    case 0x87: case 0x17: case 0x07: case 0xA7:
			switch(filetype){
			    case 3:
				b_data[1] = b_data[0];
				b_data[0] = 0x00;
			}
			d_type  %= 0x10;
			
			if (maxwav < (b_data[2]*0x100 + b_data[1])){
				maxwav  = b_data[2]*0x100 + b_data[1];
			}
			
			if((b_data[2]*0x100 + b_data[1]) == 1)
			{
				if(wav01_auto)
					break;
				else
					wav01_auto++;
			}
 
			dataWritingUnit(dif);
			break;
			
			
		  /* BPM変更 */
		    case 0x04: case 0x14: case 0x24: case 0x34:
			b_data[2] = d_type / 0x10;
			b_data[1] = b_data[0];
			b_data[0] = 0x01;
			d_type   %= 0x10;
			dataWritingUnit(dif);
			
			start_flag++;
			
			switch(filetype){
			    case 3:
				if(rtm_flag == 0){
					d_type    = 0x05;
					b_data[0] = 0x04;
					b_data[1] = 0x04;
					b_data[2] = 0x00;
					dataWritingUnit(dif);
					rtm_flag++;
				}
				break;
			}
			break;
			
		  /* 拍子変更 */
		    case 0x05:
			switch(filetype){
			    case 3:
				break;
			    default:
				dataWritingUnit(dif);
			}
			break;
			
		  /* 譜面終了宣言 */
		    case 0x06:
			if(dp_flag == 3){
				b_data[0] = 0x01;
			}
			dataWritingUnit(dif);
			
			if((dp_flag == 0)||(dp_flag == 3)){
				temp = 0x7FFFFFFF;
				fwrite(&temp, 1, 4, out);
				temp = 0x00;
				for(i=0;i<0x04;i++){
					fwrite(&temp, 1, 1, out);
				}
				f_size[dif] += 8;
			}
			dp_flag += 2;
			break;
			
		  /* 単位時間判定 */
		    case 0x2B:
			t_base_type = 1;
			break;
			
		    case 0x4C:
			break;
			
		    default:
			printf("Unknown data type! : %06x : %02x (%02x %02x %02x)\n",i_timeA ,d_type, b_data[0], b_data[1], b_data[2]);
			break;
		}
		//printf("DEBUG: %06x : %02x (%02x %02x %02x); %d\n",i_timeA ,d_type, b_data[0], b_data[1], b_data[2], dp_flag);
	}
	
    /* サブヘッダのTotalNotes書き換え */
	fseek(out, head_adrs + 0x06, SEEK_SET);
	fwrite(&t_notes[dif], 1, 2, out);
	
    /* ヘッダ書き換え */
	headWritingUnit(dif);
	
	return 0;
}

/* .1ヘッダ生成 */
int makeHeader(void){
	int i;
	int temp;
    /* ヘッダ仮置き */
	temp = 0x00;
	for(i=0;i<0x60;i++){
		fwrite(&temp, 1, 1, out);
	}
	return 0;
}

/* CS譜面ファイル確認 */
int checkScoreFile(void){
	int	i,j;
	int	existflag;
	int	headerflag;
	char	f_type[NAMEMAX];
	
	filetype   = 0;
	headerflag = 0;
	
    /* CS3ファイル */
	if((in = fopen("cs3.raw", "rb")) == NULL) {
		printf("cs3.raw : Not Found.\n\n");
	}
	else {
		printf("cs3.raw : Read success / ");
		filetype = 1;
		
	    /* DJT,GOLD判別 */
		fseek(in, 0, SEEK_END);
		if (ftell(in) / 0x10000 == 6){
			f_exist[3] = -1;
			printf("CS3 type : GOLD\n\n");
		}
		else {
			printf("CS3 type : DJ TROOPERS\n\n");
		}
		
	    /* 譜面存在フラグ検出 */
		printf("Prosessing ");
		for(j=0;j<7;j++){
		    /* フラグ読み込み */
			existflag = 0;
			fseek(in, 0x10000 * j, SEEK_SET);
			fread(&existflag, 1, 1, in);
			fseek(in, 0x10000 * j, SEEK_SET);
			
		    /* GOLDの場合SPB分を飛ばす */
			if((j >= 3)&&(f_exist[3] == -1)){
				j++;
			}
			
		    /* フラグ判定 */
			if(existflag == 0x00){	// 存在しない
				f_exist[j] = 0;
			}
			else {			// 存在する
			    /* ヘッダ生成 */
				if(headerflag == 0){
					makeHeader();
					headerflag++;
				}
			    /* 変換処理 */
				printf("-");
				cs2ac(j);
				printf("> ");
				f_exist[j] = 1;
			}
			
		    /* GOLDの場合ループカウンタを戻す */
			if((j >= 3)&&(j != 6)&&(f_exist[3] == -1)){ j--; }
		}
		printf("OK\n");
	}
	
    /* CS,CS2,CS5ファイル */
	for(i=2;i<=3;i++){
	    if(filetype != 0){
		break;
	    }
	    
	    switch(i){
		case 2: sprintf(f_type, "cs");  break; // 7th style - DistorteD
		case 3: sprintf(f_type, "cs2"); break; // 3rd style - 6th style
//		case 4: sprintf(f_type, "cs5"); break; // ??
	    }
	    for(j=0;j<7;j++){
		switch(j){
		    case 0: sprintf(inName, "n7.%s",f_type);  break;
		    case 1: sprintf(inName, "h7.%s",f_type);  break;
		    case 2: sprintf(inName, "a7.%s",f_type);  break;
		    case 3: sprintf(inName, "b.%s",f_type);   break;
		    case 4: sprintf(inName, "n14.%s",f_type); break;
		    case 5: sprintf(inName, "h14.%s",f_type); break;
		    case 6: sprintf(inName, "a14.%s",f_type); break;
		}		
		if((in = fopen(inName, "rb")) == NULL) {
			printf("%7s : Not Found.\n",inName);
		}
		else {
			printf("%7s : Prosessing ",inName);
			filetype   = i;
		    /* ヘッダ生成 */
			if(headerflag == 0){
				makeHeader();
				headerflag++;
			}
		    /* 変換処理 */
			printf("-");
			cs2ac(j);
			f_exist[j] = 1;
			fclose(in);
			printf("> OK\n");
		}
	    }
	    printf("\n");
	}
	return filetype;
}

int main(){
	char wait[0xff];
	head_adrs = 0x60;
	maxwav    = 0;
	
	printf(" -* beatmaniaIIDX CS to AC Chart Converter v%.2f by 697@14 *-\n\n",version);
	
	if((out = fopen("output.1", "wb")) == NULL) {
		printf("Failed to open \"output.1\".\n");
		return 1;
	}
	
	if(checkScoreFile() == 0) {
		printf("Chart File not found...\n");
		return 1;
	}
	else {
		printf("Number of wav : %d\n\n",maxwav);
		printf("Writing \"output.1\"\n\nDone.\n");
	}
	fclose(out);
	
	//gets(wait);
	return 0;
}



