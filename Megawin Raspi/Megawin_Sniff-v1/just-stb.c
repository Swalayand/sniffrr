#include <bcm2835.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>


//Compile $ gcc megawin.c -l rt -l bcm2835

#define AMAX 50000000

#define STB  17 

typedef struct  {
    uint8_t state;
    int     ctr ;
} state_t;

state_t asta[AMAX];

char *state[] = { "LOW", "RISING", "FALLING", "HIGH", "BLANK" };

int sta_d[] = {1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1};

uint8_t sta_d_i, sta_prev = 0, sta_state;
int sta_counter, fallctr = 0;
uint8_t shbf, shbf_done = 0; // STB high before falling

FILE *fp;
char buf[10000];

int main(int argc, char **argv){
	
	fp = fopen("out.txt", "r");
	if (!bcm2835_init()) return 1;
    bcm2835_gpio_fsel(STB, BCM2835_GPIO_FSEL_INPT); 
    
	// Set state stack all to BLANK 
	for(int i = 0; i < AMAX; i++) { 
		asta[i].state = 4; asta[i].ctr = 0;
	}
	int i=0, j=0;
	//while ( !feof( fp )  ) {
	for(int i = 0; i < AMAX; i++) { 

		//sta_d_i = sta_d[i];
		sta_d_i = bcm2835_gpio_lev( STB );
	  //fread( buf, 1, 100, fp ); j++;
	  //if( j > 8000 ) break;
	  /*
	  for ( i=0; i<100; i++ ) { // < AMAX) {
		if ( buf[i%100] == 48 || buf[i%100] == 49 ) {
		  sta_d_i = buf[i%100]-48;
		  //printf("%d ", sta_d_i );
	    }
		*/
		if (i == 0 && sta_d_i == 1) {
			sta_state = 3;
			asta[sta_counter].state = sta_state;
			sta_prev = sta_d_i;
			continue;
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
	 //} // for setelah read
    }
	
	float us=0, ms = 0;
	for(int i = 0; i < AMAX; i++) { 
		us = asta[i].ctr*0.1;
		ms = us / 1000 ;
		if( asta[i].state == 4 ) break;
		if( asta[i].ctr  > 200000 ) fallctr = 0;
		printf("\n%2d  %3d %7s %6d %7.1f us %9.6f ms", i+1, fallctr++, state[asta[i].state], asta[i].ctr, us, ms );
	}
	
	// fclose( fp );
	bcm2835_close();
	return 0;
}
