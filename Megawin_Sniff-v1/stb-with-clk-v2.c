#include <bcm2835.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/*
 Fungsi utama: count dan shift state
 1. Menggabungkan bit saat ini (current_p) dengan sebelumnya (g_prev) untuk mendapatkan state. Count mencatat seberapa banyak state itu terdapat dalam satuan data.
 2. Membedakan state yang sudah tersimpan (pin->state) dengan yang saat ini (state_p).
*/

#define STB  17
#define CLK  18 
#define DIO  22 

typedef struct  {
    uint8_t state;
    int     ctr ;
} state_t;

#define LENGTH 10000000
uint8_t ar[LENGTH];

#define FALLING 2
#define RISING 1

char *state[] = { "LOW", "RISING", "FALLING", "HIGH", "BLANK" };

state_t stb_pin, stb_pin_prev, clk_pin, clk_pin_prev, dio_pin;
uint8_t g_s_prev, g_c_prev, g_d_prev; // general stb/clk/dio prev

int shbf_done = 0;

uint8_t set_state(uint8_t current_p, uint8_t prev_p, state_t *pin, uint8_t *gp, state_t *pp){
	uint8_t state_p;
	state_p = current_p | (prev_p << 1);
	
	pp->state = pin->state;
	pp->ctr = pin->ctr;
	if (state_p != pin->state){
		pin->state = state_p;
		pin->ctr=1;
	}else{
		pin->ctr++;
	}
	
	*gp = current_p; // general prev
	return state_p;
}

int main(){
	
	if (!bcm2835_init()) return 1;
    bcm2835_gpio_fsel(STB, BCM2835_GPIO_FSEL_INPT); 
	bcm2835_gpio_fsel(CLK, BCM2835_GPIO_FSEL_INPT); 
    bcm2835_gpio_fsel(DIO, BCM2835_GPIO_FSEL_INPT); 
	
	uint8_t stb_dat, stb_state, clk_dat, clk_state, dio_dat;
	int ctr;
	uint8_t value = 0;

	for (int i = 0; i < LENGTH; i++){
		
		stb_dat = bcm2835_gpio_lev( STB );
		clk_dat = bcm2835_gpio_lev( CLK );
		stb_state = set_state(stb_dat, g_s_prev, &stb_pin, &g_s_prev, &stb_pin_prev);
		clk_state = set_state(clk_dat, g_c_prev, &clk_pin, &g_c_prev, &clk_pin_prev);
		
		if (stb_pin_prev.ctr > 40000 && stb_pin.state == FALLING){
			shbf_done = 1;
		}

		if (shbf_done == 1 && clk_pin.state == RISING){
			//ctr++;
			//printf("%8s %8d %d %d %6d\n", state[clk_pin.state], i, g_c_prev, clk_pin.ctr, ctr );
			for (int i = 0; i < 8; i++){
				dio_dat = bcm2835_gpio_lev( DIO );
				value |= dio_dat << i;
			}
			printf("%d", dio_dat);
		}
	}
	
	/*
	for (int i = 0; i < LENGTH; i++){
		printf("%d", ar[i]);
	}
	*/

	return 0;
}
