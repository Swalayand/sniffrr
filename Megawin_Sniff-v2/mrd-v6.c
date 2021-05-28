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

void* printEvent(void *arg){	
	/*
	if (ar[0] == 3 && ar[1] == 64){
		printf("0: %3d 1:%3d 2:%3d \n", ar[0], ar[1], ar[2]); fflush( stdout );
	}
	*/
	while(1){
		if (car < 5) continue;
		for (int i = 1; i < car; i++){	
			for (int j = 0; j < 20; j++)
				if (ar[i] == full_digits[j]) {
					printf("%d ", j%10 );
				}
		}
		printf("\r"); fflush( stdout );
		sleep(0.2);
		car = 0;
	}

}

void eventCount(){
	uint8_t stb_dat, stb_state, clk_dat, clk_state, dio_dat, ctr_dio = 0, value = 0;
	int count_rising_stb = 0,shbf_done = 0;
	while(1){
		stb_dat = bcm2835_gpio_lev( STB );
		clk_dat = bcm2835_gpio_lev( CLK );
		stb_state = setState(stb_dat, g_s_prev, &stb_pin, &g_s_prev, &stb_pin_prev);
		clk_state = setState(clk_dat, g_c_prev, &clk_pin, &g_c_prev, &clk_pin_prev);
		if (stb_pin_prev.ctr > 50000 && stb_pin.state == FALLING) shbf_done = 1;			
		if (shbf_done == 1 && clk_pin.state == RISING){
			dio_dat = bcm2835_gpio_lev( DIO );
			if (ctr_dio > 7){
				if (value > 114) ar[car++] = value;
				ctr_dio = 0; value = 0; 
			}
			value = value | (dio_dat << ctr_dio++);
		}
	}
}
/*
 eventCount mengumpulkan bit sampai jadi decimal. Decimalnya kumpulin di array.
 Print decimal yang di array ketika :
 a. STB persiapan ke cycle berikutnya
 b. jumlah array sudah mencapai jumlah sevseg
 
 Problemnya, ngeprint nggak konstan (ketambahan angka yang tidak seharusnya).
 Angka yang panjangnya nggak sesuai sevseg tetep diprint, dan ditimpakan ke ujung kiri. 
 Atau ketika panjangnya melebihi sevseg, juga tetep diprint. Jadi kelihatan panjang banget outputnya. Dan nggak sesuai.

 Untuk mencegah kejadian itu:
 1. Agka yang perlu diprint adalah angka yang ada di antara header dan footer 
 2. Kalau angka ke 18 adalah 138 dan angka pertamanya 3, print angka ditengahnya. Atau kalau angka pertama bukan 3 dan angka ke 18 bukan 138 abaikan sisanya. 
 3. CAR ditambahkan hanya ketika valuenya lebih dari 114 (masuk dalam digits).
*/
