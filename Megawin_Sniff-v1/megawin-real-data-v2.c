#include <bcm2835.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

/*
 Fungsi utama: count dan shift state
 1. Membandingkan bit saat ini dengan sebelumnya untuk mendapatkan state. Count mencatat seberapa banyak state itu terdapat dalam satuan data.
*/
#define STB  17 

//       index      0        1         2         3        4  
//      binary     00       01        10        11      100  
char *state[] = { "LOW", "RISING", "FALLING", "HIGH", "BLANK" };


uint8_t sta_d[] = {1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1};

typedef struct  {
    uint8_t state;
    int     ctr ;
} state_t;

state_t asta[10000000];

state_t pin1;

int shbf;

uint8_t g_prev;

int j = 0;
void set_state(uint8_t current_p, uint8_t prev_p, state_t *pin){
	uint8_t state_p;
	state_p = current_p | (prev_p << 1);
	if (current_p == prev_p){
		if (state_p == 3) shbf++;
		pin->state = state_p;
		pin->ctr++;
	}else{
		//printf("%5s %d\n", state[pin->state], pin->ctr);
		asta[j].state = state_p;
		asta[j].ctr = pin->ctr;
		j++;
		if (shbf > 200000) printf("One Cycle\n");
		if (state_p == 1){
			//printf("%5s\n", state[state_p]);
			asta[j].state = state_p;
			asta[j].ctr = pin->ctr;
			j++;
		}else if (state_p == 2){
			//printf("%5s\n", state[state_p]);
			asta[j].state = state_p;
			asta[j].ctr = pin->ctr;
			j++;
		}
		pin->state = state_p;
		pin->ctr = 0;
		shbf=0;
	}
	g_prev = current_p;
}

int main(){
	if (!bcm2835_init()) return 1;
    bcm2835_gpio_fsel(STB, BCM2835_GPIO_FSEL_INPT); 
	
	//for (int i = 0; i < 18+7; i++){
	for (int i = 0; i < 10000000; i++){
		int data_i;
		//data_i = sta_d[i];
		data_i = bcm2835_gpio_lev( STB );
		if (i == 0 && data_i == 1){
			set_state(data_i, 1, &pin1);
			continue;
		}else if (i == 0 && data_i == 0){
			set_state(data_i, 0, &pin1);
			continue;
		}
		set_state(data_i, g_prev, &pin1);
	}
	for (int i = 0; i < 10000000; i++){
		printf("%s %d\n",state[asta[i].state], asta[i].ctr);
	}
	bcm2835_close();
	return 0;
}
