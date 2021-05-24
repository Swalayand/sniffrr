#include <bcm2835.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define STB  17
#define CLK  18 
#define DIO  22 
#define LENGTH 10000000
#define FALLING 2
#define RISING 1

typedef struct  {
    uint8_t state;
    int     ctr ;
} state_t;
uint8_t ar[LENGTH];
                           /*0*//*1*//*2*//*3*//*4*//*5*//*6*//*7*//*8*//*9*/
uint8_t digits[]       = { 219, 130, 185, 179, 226, 115, 123, 195, 251, 243 };
uint8_t digits_comma[] = { 223, 134, 191, 183, 230, 119, 127, 199, 255, 247 };

char *state[] = { "LOW", "RISING", "FALLING", "HIGH", "BLANK" };
state_t stb_pin, stb_pin_prev, clk_pin, clk_pin_prev, dio_pin;
uint8_t g_s_prev, g_c_prev; // general stb/clk/dio prev

uint8_t set_state(uint8_t current_p, uint8_t prev_p, state_t *pin, uint8_t *gp, state_t *pp){
	uint8_t state_p;
	state_p = current_p | (prev_p << 1);
	pp->state = pin->state;
	pp->ctr = pin->ctr;
	if (state_p != pin->state){
		pin->state = state_p;
		pin->ctr = 1;
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
	uint8_t stb_dat, stb_state, clk_dat, clk_state, dio_dat, ctr_dio = 0, value = 0;
	int count_rising_stb = 0, car = 0,shbf_done = 0;
	for (int i = 0; i < LENGTH; i++){
		stb_dat = bcm2835_gpio_lev( STB );
		clk_dat = bcm2835_gpio_lev( CLK );
		stb_state = set_state(stb_dat, g_s_prev, &stb_pin, &g_s_prev, &stb_pin_prev);
		clk_state = set_state(clk_dat, g_c_prev, &clk_pin, &g_c_prev, &clk_pin_prev);
		if (stb_pin_prev.ctr > 50000 && stb_pin.state == FALLING) shbf_done = 1;			
		if (shbf_done == 1 && clk_pin.state == RISING){
			dio_dat = bcm2835_gpio_lev( DIO );
			if (ctr_dio > 7){
				ar[car++] = value;
				if (car > 150) break;
				ctr_dio = 0; value = 0; 
			}
			value = value | (dio_dat << ctr_dio++);
			ar[car++] = dio_dat;
		}	
	}
	printf("  1   2   3   4   5   6   7   8\n\n");
	for (int i = 0; i < car; i++){
	printf("%3d ", ar[i]);
		if ((i+1)%9 == 0 && i!=0){
			for (int j = 0; j < 10; j++)
				if (ar[i] == digits[j]) printf("%d ", j%10 );
			printf("\n");
		}
	}		
	printf("\n");
	return 0;
}
