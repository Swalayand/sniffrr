#include <stdint.h>
#include <stdio.h>
#include <time.h>

//Compile $ gcc megawin.c -l rt -l bcm2835

#define AMAX 18+6

typedef struct  {
    uint8_t state;
    int     ctr ;
} state_t;

state_t asta[AMAX];

char *state[] = { "LOW", "RISING", "FALLING", "HIGH", "BLANK" };

int sta_d[] = {1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1};

uint8_t sta_d_i, sta_prev=0, sta_state, sta_counter;
uint8_t shbf, shbf_done = 0; // STB high before falling
int main(int argc, char **argv){
    
  // Set all to BLANK 
	for(int i = 0; i < AMAX; i++) { 
		asta[i].state = 4; asta[i].ctr = 0;
	}
	
	for (int i = 0; i <= AMAX; i++){
		sta_d_i = sta_d[i];

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
			printf("%d STB high before falling edge\n", i);
		}
		
		sta_prev = sta_d_i;
		
	}
	
	for(int i = 0; i < AMAX; i++) { 
		printf("%2d %7s %d\n", i+1, state[asta[i].state], asta[i].ctr);
	}

  return 0;
}
