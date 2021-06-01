#include <bcm2835.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>  // gcc thread1.c -lpthread 

#define STB  17
#define CLK  18 
#define DIO  22 
#define LENGTH 100
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
uint8_t g_s_prev, g_c_prev; // general stb/clk prev

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

void* printEvent(void *);
void eventCount();
int car = 0;

int main(){
	if (!bcm2835_init()) return 1;
    bcm2835_gpio_fsel(STB, BCM2835_GPIO_FSEL_INPT); 
	bcm2835_gpio_fsel(CLK, BCM2835_GPIO_FSEL_INPT); 
    bcm2835_gpio_fsel(DIO, BCM2835_GPIO_FSEL_INPT); 
	pthread_t t;
	pthread_create( &t, NULL, printEvent, NULL ); 
	eventCount();
	return 0;
}

int shbf_done = 0;

void* printEvent(void *arg){	
	while(1){
		sleep(0.1);
		if (car < 54) continue;
		car = 0;
		if (ar[17] != 138 || ar[5] < 115 ) continue;
		for (int i = 0; i < 18; i++){	
			for (int j = 0; j < 20; j++)
				if (ar[i] == full_digits[j] && ar[13] == 3) printf("%d ", j%10 ); 
		}
		printf("\r"); fflush( stdout );
	}

}

void eventCount(){
	uint8_t stb_dat, stb_state, clk_dat, clk_state, dio_dat, ctr_dio = 0, value = 0;
	while(1){
		stb_dat = bcm2835_gpio_lev( STB );
		clk_dat = bcm2835_gpio_lev( CLK );
		stb_state = setState(stb_dat, g_s_prev, &stb_pin, &g_s_prev, &stb_pin_prev);
		clk_state = setState(clk_dat, g_c_prev, &clk_pin, &g_c_prev, &clk_pin_prev);
		
		if (stb_pin_prev.ctr > 40000 && stb_pin.state == FALLING) shbf_done = 1;			
		if (shbf_done == 1 && clk_pin.state == RISING){
			dio_dat = bcm2835_gpio_lev( DIO );
			if (ctr_dio > 7){
				ar[car++] = value;
				ctr_dio = 0; value = 0; 
			}
			value = value | (dio_dat << ctr_dio++);
			if ( (car+1) % 54 == 0 && car != 0 ) shbf_done = 0;
			
		}
	}
}
/*
Algoritma yang kelihatannya sudah benar, gambar saleae yang udah bagus banget, kalimat yang sulit diimprove, membuat LOG menjadi pilihan 
proses membentuk pemahaman yang utuh. 

Dimulai dengan keinginan: LOG print 3,64,192 sebagai awal dari baris secara konsisten berkali-kali


LOG 1 banyak benernya tapi masih ada yang meleset

  3  64 192   0   0   0   0   0   0   0   0   0 128 109   0   0   0 255 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 
  3  64 192   0   0   0   0   0   0   0   0 219   0 219 128   0   0 197   1  32  96   0   0   0   0   0   0   0   0   0 128 109   0   0   0 255 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 
 
LOG 2 dengan penyesuaian tempat if ( car % 54 == 0 && car != 0 ) shbf_done = 0; dan diubah menjadi if ( (car+1) % 54 == 0 && car != 0 ) shbf_done = 0; menjadi konsisten

  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 224   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 
  3  64 192   0   0 134   0 185   0 243   0 115   0   2   0   0   0 138 1 2 9 5 

Baru setelah itu print-nya dibuat mirip dengan timbangan (pakai print("\r");)
*/
