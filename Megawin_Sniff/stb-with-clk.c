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

state_t ar[10000000];
state_t pin1;

char *state[] = { "LOW", "RISING", "FALLING", "HIGH", "BLANK" };

int shbf, shbf_done;

uint8_t g_prev;

int j = 0;
void set_state(uint8_t current_p, uint8_t prev_p, state_t *pin){
	uint8_t state_p;
	state_p = current_p | (prev_p << 1);
	
	if (state_p != pin->state){

		ar[j].state = pin->state;
		ar[j].ctr = pin->ctr;
		j++;
		pin->state = state_p;
		pin->ctr=1;
	}else{
		if (state_p == 3){
			shbf++;
		}
		ar[j].ctr = pin->ctr;
		pin->ctr++;
	}
	
	//printf("%7s\n", state[state_p]);
	g_prev = current_p;
}

int main(){
	
	if (!bcm2835_init()) return 1;
    bcm2835_gpio_fsel(STB, BCM2835_GPIO_FSEL_INPT); 
	

	for (int i = 0; i < 10000000; i++){
		ar[i].state = 4;
		ar[i].ctr = 0;
	}
	
	uint8_t dat;
	for (int i = 0; i < 10000000; i++){
		dat = bcm2835_gpio_lev( STB );
		set_state(dat, g_prev, &pin1);
	}
	

	int jum = 0, cycle; 
	for (int i = 0; i < 10000000; i++){

		jum += ar[i].ctr;
		if (ar[i].state == 4){
			break;
		}
		if (ar[i].ctr > 200000){
			shbf_done = 1;
			cycle = 0;
		}
		if (shbf_done == 1 && ar[i].state == 2){
			shbf_done = 0;
			printf("\nOne cycle\n");
		}
		
		printf("%3d %2d %7s %d\n", i, cycle, state[ar[i].state], ar[i].ctr);
		cycle++;
	}
	
	printf("Complete with %d data", jum);
	printf("\n");

	return 0;
}
