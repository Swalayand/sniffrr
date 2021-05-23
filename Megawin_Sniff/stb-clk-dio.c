#include <bcm2835.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/*
 Fungsi utama: count dan shift state
 1. Menggabungkan bit saat ini (current_p) dengan sebelumnya (g_prev) untuk mendapatkan state. Count mencatat seberapa banyak state itu terdapat dalam satuan data.
 2. Membedakan state yang sudah tersimpan (pin->state) dengan yang saat ini (state_p).
 Yang ditandai pakai #1 berarti satu algoritma yang saling berhubungan:
 1. Ketika stb_pin.ctr sudah mencapai >40000 maka ctr selanjutnya akan ditimpa (diganti dengan pembacaan baru) karena pakai struct biasa (nggak disimpan di array).
	Sehingga pengecekan akan masa lalu dan masa sekarang sangat penting. Maka muncul stb_pin_prev.ctr.

*/

#define STB  17
#define CLK  18 
#define DIO  22 

typedef struct  {
    uint8_t state;
    int     ctr ;
} state_t;

#define LENGTH 10000000
#define FALLING 2
#define RISING 1

                           /*0*//*1*//*2*//*3*//*4*//*5*//*6*//*7*//*8*//*9*/
uint8_t digits[]       = { 219, 130, 185, 179, 226, 115, 123, 195, 251, 243 };
uint8_t digits_comma[] = { 223, 134, 191, 183, 230, 119, 127, 199, 255, 247 };

char *state[] = { "LOW", "RISING", "FALLING", "HIGH", "BLANK" };
					//#1								
state_t stb_pin, stb_pin_prev, clk_pin, clk_pin_prev, dio_pin;
uint8_t g_s_prev, g_c_prev; // general stb/clk/dio prev

int shbf_done = 0;

uint8_t set_state(uint8_t current_p, uint8_t prev_p, state_t *pin, uint8_t *gp, state_t *pp){
	uint8_t state_p;
	state_p = current_p | (prev_p << 1);
	
	//#1
	pp->state = pin->state;
	pp->ctr = pin->ctr;
	//
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
	
	uint8_t stb_dat, stb_state, clk_dat, clk_state, dio_dat;
	uint8_t ctr_dio = 0;
	uint8_t value   = 0;

	for (int i = 0; i < LENGTH; i++){
		
		stb_dat = bcm2835_gpio_lev( STB );
		clk_dat = bcm2835_gpio_lev( CLK );
		stb_state = set_state(stb_dat, g_s_prev, &stb_pin, &g_s_prev, &stb_pin_prev);
		clk_state = set_state(clk_dat, g_c_prev, &clk_pin, &g_c_prev, &clk_pin_prev);
		
		//#1
		if (stb_pin_prev.ctr > 50000 && stb_pin.state == FALLING){
			shbf_done = 1;
		}
		//

		if (shbf_done == 1 && clk_pin.state == RISING){
			//shbf_done = 0; kesalahan logic yang sangat fatal; shbf_done dibutuhkan berkali2, tapi dibatasi
			/*
			Kenyataanya DIO tergantung rising clk. 
			Di bawah ini hanya mengopi kode dengan mengabaikan logika yang sesuai.
			DIO perlu dishift sebanyak 8 kali rising, bukan 8 kali berturut2 seperti di
			bawah ini:
			
			for (i = 0; i < 8; ++i) {
			 	dio_dat = bcm2835_gpio_lev( DIO );
				value |= dio_dat << ctr_dio;
			}
			*/
			if (ctr_dio < 8){ 
				dio_dat = bcm2835_gpio_lev( DIO );
				value |= dio_dat << ctr_dio;
				ctr_dio++;
			}else{
				if (value != 0)
				for (int i = 0; i < 10; i++){
					if (value == digits[i]){
						printf("%d ", i);
					}
				}
				ctr_dio = 0; value = 0;
			}
		}
	}
	
	return 0;
}
