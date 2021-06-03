#include <bcm2835.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

/*
 Fungsi utama: count dan shift state
 1. Menggabungkan bit saat ini (current_p) dengan sebelumnya (g_prev) untuk mendapatkan state. Count mencatat seberapa banyak state itu terdapat dalam satuan data.
 2. Membedakan state yang sudah tersimpan (pin->state) dengan yang saat ini (state_p).
*/

#define STB  17 

typedef struct  {
    uint8_t state;
    int     ctr ;
} state_t;

// Semua dengan tanda #1 adalah kode untuk pembuktian data benar
//#1 state_t ar[10000000];
state_t pin1;

char *state[] = { "LOW", "RISING", "FALLING", "HIGH", "BLANK" };

int shbf;

uint8_t g_prev, g_s_prev;

int j = 0;
void set_state(uint8_t current_p, uint8_t prev_p, state_t *pin){
	uint8_t state_p;
	state_p = current_p | (prev_p << 1);
	
	if (state_p != pin->state){
		/* 
		#1 
		ar[j].state = pin->state;
		ar[j].ctr = pin->ctr;
		j++;
		*/
		pin->state = state_p;
		pin->ctr=1;
	}else{
		//#1 ar[j].ctr = pin->ctr;
		pin->ctr++;
	}
	
	//printf("%7s\n", state[state_p]);
	g_prev = current_p;
}

int main(){
	
	if (!bcm2835_init()) return 1;
    bcm2835_gpio_fsel(STB, BCM2835_GPIO_FSEL_INPT); 
	
	/*
	#1
	for (int i = 0; i < 10000000; i++){
		ar[i].state = 4;
		ar[i].ctr = 0;
	}
	*/
	
	uint8_t dat;
	for (int i = 0; i < 10000000; i++){
		dat = bcm2835_gpio_lev( STB );
		set_state(dat, g_prev, &pin1);
	}
	
	/* 
	#1
	int jum = 0;
	for (int i = 0; i < 10000000; i++){

		jum += ar[i].ctr;
		if (ar[i].state == 4){
			break;
		}
		
		printf("%3d %7s %d\n", i, state[ar[i].state], ar[i].ctr);
	}
	printf("%d", jum);
	*/
	printf("\n");

	return 0;
}
