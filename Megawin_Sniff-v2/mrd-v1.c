#include <bcm2835.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define STB  	17
#define CLK  	18 
#define DIO 	22 
#define LENGTH 	10000000
#define FALLING 2
#define RISING  1

typedef struct  {
    uint8_t state;
    int     ctr ;
} state_t;
uint8_t ar[LENGTH];
																		   /*0*//*1*//*2*//*3*//*4*//*5*//*6*//*7*//*8*//*9*/
uint8_t full_digits[] = { 223, 134, 189, 183, 230, 119, 127, 199, 255, 247, 219, 130, 185, 179, 226, 115, 123, 131, 251, 243 };

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
	}else pin->ctr++;
	*gp = current_p; // general prev
	return state_p;
}

void event_count();
int car = 0;

int main(){
	if (!bcm2835_init()) return 1;
    bcm2835_gpio_fsel(STB, BCM2835_GPIO_FSEL_INPT); 
	bcm2835_gpio_fsel(CLK, BCM2835_GPIO_FSEL_INPT); 
    bcm2835_gpio_fsel(DIO, BCM2835_GPIO_FSEL_INPT); 
	
	ec:
	event_count();
	
	int proc_ar = 0;
	
	for (int i = 0; i < car; i++){
		printf("%3d ", ar[i]);
		if ((i+1)%9 == 0 && i!=0){
			for (int j = 0; j < 20; j++)
				if (ar[i] == full_digits[j]) {
					printf("%d", j%10 );
					proc_ar++;
				}
			printf("\n");
		}
	}
	if (proc_ar < 3) {
		//sleep(0.1);
		car = 0;
		goto ec;
	}
	printf("\n");
	return 0;
}

void event_count(){
	uint8_t stb_dat, stb_state, clk_dat, clk_state, dio_dat, ctr_dio = 0, value = 0;
	int count_rising_stb = 0,shbf_done = 0;
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
}

/*
 1   1   0   0   0   0   0   0   3  
 0   0   0   0   0   0   1   0  64 
 0   0   0   0   0   0   1   1 192 
 0   0   0   0   0   0   0   0   0 
 0   0   0   0   0   0   0   0   0 
 0   1   1   0   0   1   1   1 230 4 
 0   0   0   0   0   0   0   0   0 
 1   1   0   1   1   1   1   1 251 8 
 0   0   0   0   0   0   0   0   0 
 1   1   0   0   0   0   0   1 131 7 
 0   0   0   0   0   0   0   0   0 
 1   1   0   1   1   0   1   1 219 0 
 0   0   0   0   0   0   0   0   0 
 0   1   0   0   0   0   0   0   2 
 0   0   0   0   0   0   0   0   0 
 0   0   0   0   0   0   0   0   0 
 0   0   0   0   0   0   0   0   0 

 1   1   0   0   0   0   0   0   3 
 0   0   0   0   0   0   1   0  64 
 0   0   0   0   0   0   1   1 192 
 0   0   0   0   0   0   0   0   0 
 0   0   0   0   0   0   0   0   0 
 0   0   0   1   1   1   0   0  56 
 0   0   0   0   0   0   1   1 192 
 0   1   1   1   1   1   0   0  62 
 0   0   0   0   0   0   1   1 192 
 0   0   0   0   0   1   0   0  32 
 0   0   0   0   0   0   1   1 192 
 0   1   1   0   1   1   0   0  54 
 0   0   0   0   0   0   0   1 128 
 0   0   0   0   0   0   0   0   0 
 0   0   0   0   0   0   0   0   0 
 0   0   0   0   0   0   0   0   0 
 0   0   0   0   1   0   1   0  80
   
 1   1   0   0   0   0   0   0   3 
 0   0   0   0   0   1   0   0  32 
 0   0   0   0   0   1   1   0  96 
 0   0   0   0   0   0   0   0   0 
 0   0   0   0   0   0   0   1 128 
 1   1   1   1   0   1   1   0 111 
 0   0   0   0   0   0   0   0   0 
 1   0   0   0   0   0   1   0  65 
 0   0   0   0   0   0   0   1 128 
 1   0   0   0   0   0   1   0  65 
 0   0   0   0   0   0   0   1 128 
 1   0   1   1   0   1   1   0 109 
 0   0   0   0   0   0   0   0   0 
 1   0   0   0   0   0   0   0   1 
 0   0   0   0   0   0   0   0   0 
 0   0   0   0   0   0   0   0   0 
 0   0   0   0   0   0   0   0   0 

  1   1   0   0   0   0   0   0   3 
  0   0   0   0   0   0   1   0  64 
  0   0   0   0   0   0   1   1 192 
  0   0   0   0   0   0   0   0   0 
  0   0   0   0   0   1   0   1 160 
  1   1   1   0   1   0   0   0  23 
  0   0   0   0   0   1   1   0  96 
  1   1   0   1   1   0   0   0  27 
  0   0   0   0   0   1   1   0  96 
  0   1   1   1   1   0   0   0  30 
  0   0   0   0   0   1   1   0  96 
  1   1   0   1   0   0   0   0  11 
  0   1   0   0   0   0   0   0   2 
  0   0   0   0   0   0   0   0   0 
  0   0   0   0   0   0   0   0   0 
  0   0   0   0   0   0   0   0   0 
  0   1   0   1   0   0   0   1 138 
  1   1   0   0   0   0   0   0   3 
  0   0   0   0   0   0   1   0  64 
  0   0   0   0   0   0   1   1 192 
  0   0   0   0   0   0   0   0   0 
  0   0   0   0   0   0   0   0   0 
  1   0   1   1   1   1   0   1 189 2
  0   1   1   0   1   1   0   0  54 
  0   0   0   0   0   0   1   1 192 
  0   0   1   1   1   1   0   0  60 
  0   0   0   0   0   0   1   1 192 
  0   1   1   0   1   1   0   0  54 
  0   0   0   0   0   0   0   1 128 
  0   0   0   0   0   0   0   0   0 
  0   0   0   0   0   0   0   0   0 
  0   0   0   0   0   0   0   0   0 
  0   0   0   0   0   0   0   1 128 
  0   1   0   0   0   1   1   1 226 4
  1   1   0   0   0   0   0   0   3 
  0   0   0   0   0   0   1   0  64 
  0   0   0   0   0   0   1   1 192 
  0   0   0   0   0   0   0   0   0 
  0   0   0   0   0   0   0   0   0 
  1   0   1   1   1   1   0   1 189 2
  0   0   0   0   0   0   0   0   0 
  1   1   0   1   1   0   1   1 219 0
  0   0   0   0   0   0   0   0   0 
  1   1   0   0   1   1   1   1 243 9
  0   0   0   0   0   0   0   0   0 
  1   1   0   1   1   0   1   1 219 0
  0   0   0   0   0   0   0   0   0 
  0   1   0   0   0   0   0   0   2 
  0   0   0   0   0   0   0   0   0 
  0   0   0   0   0   0   0   0   0 
  0   0   0   0   0   0   0   0   0 

  Bisa jadi algoritmanya dah bener, buat mau maju ke step lain/lanjut. 8:30
  * 
*/

