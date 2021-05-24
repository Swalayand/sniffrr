#include <bcm2835.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define LENGTH 8000000


typedef struct  {
    uint8_t state;
    int     ctr ;
} state_t;

state_t asta[LENGTH];

state_t pin1;

char *state[] = { "LOW", "RISING", "FALLING", "HIGH", "BLANK" };


int shbf;

uint8_t g_prev;

void set_state(uint8_t current_p, uint8_t prev_p, state_t *pin){
	int j = 0;
	uint8_t state_p;
	state_p = current_p | (prev_p << 1);
	if (current_p == prev_p){
		if (state_p == 3) shbf++;
		pin->state = state_p;
		pin->ctr++;
	}else{
		//printf("%5s %d\n", state[pin->state], pin->ctr);
		//asta[j].state = state_p;
		//asta[j].ctr = pin->ctr;
		//j++;

		if (state_p == 1){
			printf("%5s\n", state[state_p]);
			//asta[j].state = state_p;
			//asta[j].ctr = pin->ctr;
			//j++;
		}else if (state_p == 2){
			printf("%5s\n", state[state_p]);
			//asta[j].state = state_p;
			//asta[j].ctr = pin->ctr;
			//j++;
		}
		pin->state = state_p;
		pin->ctr = 0;
		shbf=0;
	}
	g_prev = current_p;
}

int main(){
	
	//if (!bcm2835_init()) return 1;
   // bcm2835_gpio_fsel(17, BCM2835_GPIO_FSEL_INPT); 
	
	/*
	uint8_t ar[LENGTH+1];    
    int j =0;
    for (int i = 0; i < LENGTH; i++){
		ar[i] = bcm2835_gpio_lev( 17 );
	}
	
	while (j++ < LENGTH){
		printf("%d", ar[j]);
	}
	*/
	
	FILE *fp = fopen( "./ehlo.txt" , "r" ) ;
	if( fp == NULL ) printf( "error failed\n" );
	char buf[100]; buf[100]='\0';
	printf( "fopen return = %d\n", fp );
	int i = 0; // kalo nggak ada variable ini, apapun namanya, maka koding ini akan crash.
	while (!feof(fp)){
		fread( buf, 1,99, fp );
		printf("%s", buf );
		for ( int i = 0; i < 99; i++){
			printf("%d", buf[i]-48);
		}
		fflush( stdout );
	}
	printf("\n");
	
	//fclose( fp );
	
	//bcm2835_close();
	return 0;
}
