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

state_t stb_pin;

char *state[] = { "LOW", "RISING", "FALLING", "HIGH", "BLANK" };

int shbf;

uint8_t g_prev;

int j = 0;
void set_state(uint8_t current_p, uint8_t prev_p, state_t *pin){
	uint8_t state_p;
	state_p = current_p | (prev_p << 1);
	
	if (state_p != pin->state){
		pin->state = state_p;
		pin->ctr=1;
		shbf = 0;
	}else{
		if (state_p == 3){
			shbf++;
		}
		pin->ctr++;
	}
	
	//printf("%7s\n", state[state_p]);
	g_prev = current_p;
}

int main(){
	
	if (!bcm2835_init()) return 1;
    bcm2835_gpio_fsel(STB, BCM2835_GPIO_FSEL_INPT); 
	
	uint8_t stb_dat;
	for (int i = 0; i < 10000000; i++){
		stb_dat = bcm2835_gpio_lev( STB );
		set_state(stb_dat, g_prev, &stb_pin);
		if (shbf > 200000){
			printf("One cycle\n");
		}
	}
	
	printf("\n");

	return 0;
}
