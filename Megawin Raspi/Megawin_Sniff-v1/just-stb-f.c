#include <bcm2835.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>


//Compile $ gcc just-stb-f.c -l bcm2835 -l rt

#define AMAX 5000000

#define STB  17 
#define CLK  18 
#define DIO  22 

typedef struct  {
    uint8_t state;
    int     ctr ;
} state_t;

state_t asta[AMAX];

char *state[] = { "LOW", "RISING", "FALLING", "HIGH", "BLANK" };

int sta_d[] = {1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1};

uint8_t sta_d_i, sta_prev = 4, sta_state;
int sta_counter, fallctr = 0;
uint8_t shbf, shbf_done = 0; // STB high before falling


char buf[10000];

void set_state(int i){
	if (i == 0 && sta_d_i == 1) {
		sta_state = 3;
		asta[sta_counter].state = sta_state;
		sta_prev = sta_d_i;
		return;
	}

	sta_state = sta_d_i | (sta_prev << 1);

	if ( sta_d_i != sta_prev ) {
		asta[sta_counter].ctr++;
		sta_counter++;
	}

	if (asta[sta_counter].state == sta_state) {
		if (sta_state == 3){
			shbf++;
		}
		asta[sta_counter].ctr++;
	}else{
		asta[sta_counter].state = sta_state;
		asta[sta_counter].ctr++;
	}
	
	if ( sta_d_i != sta_prev ) sta_counter++;

	if (shbf > 5 && sta_state == 2){
		shbf=0;
		shbf_done = 1;
	}
		
	sta_prev = sta_d_i;
}
state_t gstate = { 4, 0 };

int ictr =0;
void state_set(  ) {
	if( sta_prev == 4 ) { 
		sta_prev = sta_d_i; 
		gstate.state = sta_d_i | (sta_prev << 1); // ketika pertama kali jalan, asumsikan beda, supaya dilakukan inisiasi
	}
	sta_state = sta_d_i | (sta_prev << 1);
	if ( gstate.state == sta_state ) {
		//gstate.state = sta_state;
		gstate.ctr++;
	} else {
		float us=0, ms = 0;
		us = gstate.ctr*0.1;
		ms = us / 1000 ;
		//if ( gstate.state == 3 && gstate.ctr > 80000 )
		printf("\n%4d  %3d %7s %6d %7.1f us %9.6f ms", ictr, fallctr++, state[gstate.state], gstate.ctr, us, ms );
		//fflush( stdout );
		ictr++;
		gstate.state = sta_state;
		gstate.ctr = 1;
	}

	sta_prev = sta_d_i; // simpan bit sebelumnya di sta_prev untuk loop berikut
}// gstate.state; //

int main(int argc, char **argv){
	
	FILE *fp = fopen("./low-2696-rise-high4-fal-low-226.txt", "rb");
	if (!bcm2835_init()) return 1;
	printf( "bcm2835_init() ok\n" );
    bcm2835_gpio_fsel(STB, BCM2835_GPIO_FSEL_INPT); 
    bcm2835_gpio_fsel(CLK, BCM2835_GPIO_FSEL_INPT); 
    bcm2835_gpio_fsel(DIO, BCM2835_GPIO_FSEL_INPT);    
    
	// Set state stack all to BLANK 
	for(int i = 0; i < AMAX; i++) { 
		asta[i].state = 4; asta[i].ctr = 0;
	}
	int i=0, j=0, sta_int=0; 
	char o[2] = {0,0};
	uint8_t * psta = &sta_d_i;
	for(int i = 0; i < AMAX; i++) { 
		sta_d_i = bcm2835_gpio_lev( STB );
		//fread( (void*) psta, 1,1, fp );
		//o[0] = sta_d_i;
		//fwrite(  (void*) o, 1, 1, fp );
		state_set( );
		set_state(i);

    }
	printf( "\n\n" );
	float us=0, ms = 0;
	for(int i = 0; i < AMAX; i++) { 
		us = asta[i].ctr*0.1;
		ms = us / 1000 ;
		if( asta[i].state == 4 ) break;
		if( asta[i].ctr  > 200000 ) fallctr = 0;
		printf("\n%2d  %3d %7s %6d %7.1f us %9.6f ms", i+1, fallctr++, state[asta[i].state], asta[i].ctr, us, ms );
	}
	
	printf( "\n" );
	fclose( fp );
	bcm2835_close();
	return 0;
}
