#include <stdint.h>
#include <stdio.h>
#include <time.h>

/*
 Fungsi utama: count dan shift state
 1. Menggabungkan bit saat ini (current_p) dengan sebelumnya (g_prev) untuk mendapatkan state. Count mencatat seberapa banyak state itu terdapat dalam satuan data.
 2. Membedakan state yang sudah tersimpan (pin->state) dengan yang saat ini (state_p).
*/

typedef struct  {
    uint8_t state;
    int     ctr ;
} state_t;

state_t pin1;

char *state[] = { "LOW", "RISING", "FALLING", "HIGH", "BLANK" };

int shbf;

uint8_t g_prev, g_s_prev;

void set_state(uint8_t current_p, uint8_t prev_p, state_t *pin){
	
	uint8_t state_p;
	state_p = current_p | (prev_p << 1);
	
	if (state_p != pin->state){
		printf("%9s %d\n", state[pin->state], pin->ctr);
		pin->state = state_p;
		pin->ctr=1;
	}else{
		pin->ctr++;
	}
	
	//printf("%7s\n", state[state_p]);
	g_prev = current_p;
	g_s_prev = state_p;
}

int main(){
	
	FILE *fp = fopen( "./ten-million.txt" , "r" ) ;
	if( fp == NULL ) printf( "error failed\n" );
	char buf[100];
	
	int readed , chunk=1, batch=1;
	uint8_t dat;
	//while (!feof(fp)){
	for (int i = 0; !feof(fp); i++){
		readed = fread( buf, chunk, batch, fp ); 
		//printf("%s", buf );
		
		if( buf[0] == '\n'){
			break;
		}
		dat = buf[0]-48;
		if (i == 0 && dat == 1){
			set_state(dat, 1, &pin1);
			continue;
		}else if (i == 0 && dat == 0){
			set_state(dat, 0, &pin1);
			continue;
		}
		set_state(dat, g_prev, &pin1);
		
		//printf( " ,readed %d bytes\n", readed*chunk );
		//fflush( stdout );
		
	}
	printf("\n");

	return 0;
}
