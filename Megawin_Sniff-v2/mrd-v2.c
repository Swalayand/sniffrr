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

void eventCount();
uint8_t processEvent(uint8_t, uint8_t, uint8_t);

uint8_t setState(uint8_t current_p, uint8_t prev_p, state_t *pin, uint8_t *gp, state_t *pp){
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

int car = 0;

int main(){
	if (!bcm2835_init()) return 1;
    bcm2835_gpio_fsel(STB, BCM2835_GPIO_FSEL_INPT); 
	bcm2835_gpio_fsel(CLK, BCM2835_GPIO_FSEL_INPT); 
    bcm2835_gpio_fsel(DIO, BCM2835_GPIO_FSEL_INPT); 
	
	eventCount();
	
	int proc_ar = 0;
	/*
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
	*/

	printf("\n");
	return 0;
}

void eventCount(){
	uint8_t stb_dat, stb_state, clk_dat, clk_state, dio_dat, dio_ctr = 0, value = 0;
	int count_rising_stb = 0, shbf_done = 0;
	for (int i = 0; i < LENGTH; i++){
		stb_dat = bcm2835_gpio_lev( STB );
		clk_dat = bcm2835_gpio_lev( CLK );
		stb_state = setState(stb_dat, g_s_prev, &stb_pin, &g_s_prev, &stb_pin_prev);
		clk_state = setState(clk_dat, g_c_prev, &clk_pin, &g_c_prev, &clk_pin_prev);
		if (stb_pin_prev.ctr > 50000 && stb_pin.state == FALLING) shbf_done = 1;			
		if (shbf_done == 1 && clk_pin.state == RISING){
			dio_dat = bcm2835_gpio_lev( DIO );
			if (dio_ctr > 7){
				if (value != 0) ar[car++] = value;
				if (car > 150) break;
				dio_ctr = 0; value = 0; 
			}
			value = processEvent(value, dio_dat, dio_ctr++);
		}
	}
}

uint8_t processEvent(uint8_t val, uint8_t dat, uint8_t dat_c){
	uint8_t value;
	static uint8_t val_prev, header_start, header_count = 0;
	
	value = val | (dat << dat_c);

	if (dat_c == 7){
		if (header_start == 1) header_count++;
		if (header_count > 3){
			header_start = header_count = 0;
		}
		if (val_prev == 3 && value == 64){
			header_start = 1;
		}
		val_prev = value;
	}
	
	return value;
}

/*
  3  64 192   0   0 223   0 219   0  219   0 219   0   3   0   0   0 138 
  3  64 192   0   0   0   0   0   0    0   0 219   0 219   0   1   0 138 
  3  64 192   0   0   0   0   0   0    0   0   0   0 219   0   0   0 138 
  3  64 192   0   0 223   0 219   0  219   0 219   0   3   0   0   0 138 
  3  64 192   0   0   0   0   0   0    0   0 219   0 219   0   1   0 138 
  3  64 192   0   0   0   0   0   0    0   0   0   0 219   0   0   0 138 
  3  64 192   0   0 223   0 219   0  219   0 219   0   3   0   0   0 138 
  3  64 192   0   0   0   0   0   0    0   0 219   0 219   0   1   0 138 
  3  64 192   0   0   0   0 
  * 
  *  
  3 192 223 219 219 109 128   1 197 
  1  32  96 128 109 128 109 128 197 
  1  32  96 128 109 197   1  32  96 
128 111 128 109 128 109 128 109 128 
  1 197   1  32  96 128 109 128 109 
128 197   1  32  96 108   3  40  14 
  1   3 124   3 108   3 108   3 108 
  3  12  40  14   1   3 108   3 108 
  3   4  40  14   1   3 108   3 138 
  1  32  96 248 166  13 176  13 176 
 13  48 160  56   4  12 176  13 176 
 13  16 160  56   4  12 176  13 160 
 56   4  12 240  13 176  13 176  13 
176  13  48 160  56   4  12 216   6 
216   6   8  80  28   2   6 219 138 
  3  64 192 223 219 219 219   3 138 
  3  64 192 219 219   1 138 
  * 
  * 
  3  64 192 223 219 219 219   3 138 
  3  64 192 219 219   1 138   
  3  64 192 219 197   1  32  96 128 111 128 
109 128 109 128 109 128   1 197   1 
 32  96 128 109 128 109 128 197   1 
 32  96 128 109 197   1  32  96 128 
111 128 109 128 109 128 109 128   1 
197   1  16  48 192  54 192  54  64 
128 226  16  48 192  54 128 226  16 
 48 192  55 192  54 192  54 192  54 
192 128 226  16  48 192  54 192  54 
 64 128 226  16  48 192  54 128 226 4
  8  24 224  27  96  27  96 139 109 
128   1 197   1  32  96 128 109 128 
109 128 197   1  32  96 128 109 197 
  1  32  96 128 111 128 109 128 109 
128 109 128   1 197   1  32 
  Bisa jadi algoritmanya dah bener, buat mau maju ke step lain/lanjut. 8:30
  * 
*/

