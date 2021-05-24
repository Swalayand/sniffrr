#include <bcm2835.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

//Compile $ gcc megawin.c -l rt -l bcm2835

#define STB  17 
#define CLK  18 
#define DIO  22 

typedef struct  {
    uint8_t state;
    int     ctr ;
} state_t;

//                  0        1         2         3        4  
char *state[] = { "LOW", "RISING", "FALLING", "HIGH", "BLANK" };

uint8_t stb_val, stb_prev=0, stb_state;
int stb_counter, shbf, shbf_done = 0; // STB high before falling
uint8_t clk_val, clk_prev=0, clk_state;
int clk_counter = 0;
uint8_t dio_val, dio_prev=0, dio_state;

state_t stbs, clks;


void set_state(int i, uint8_t *val, uint8_t *prev, int *l_ctr, state_t *state_c, uint8_t l_state){
	//if (state_c == NULL) return;
	
	if (*val == 1 && i == 0) {
		l_state = 3;
		state_c->state == l_state;
		*prev = *val;
		return;
	}
		
	l_state = *val | (*prev << 1);
	
	if ( *val != *prev ) state_c->ctr++;
	
	if (state_c->state == l_state){
		state_c->ctr++;
		if (l_state == 3) shbf++;
	}else{
		printf("%20s %d\n", state[state_c->state], state_c->ctr);
		state_c->state = l_state;
		if (l_state == 0 || l_state == 3){
			state_c->ctr = 1;
		}else{
			state_c->ctr = 0;
		}
		shbf=1;
	}
	
	if ( *val != *prev ) {
		state_c->ctr++;
	}

	*prev = *val;
}


uint8_t sta_d[] = {1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1};

int main(int argc, char **argv){
	
	if (!bcm2835_init()) return 1;
    bcm2835_gpio_fsel(STB, BCM2835_GPIO_FSEL_INPT); 
    bcm2835_gpio_fsel(CLK, BCM2835_GPIO_FSEL_INPT); 
    bcm2835_gpio_fsel(DIO, BCM2835_GPIO_FSEL_INPT);
	
	int i=0;
	while(i++ < 18+7){
		//stb_val = bcm2835_gpio_lev( STB );
		stb_val = sta_d[i];
		
		set_state(i, &stb_val, &stb_prev, &stb_counter, &stbs, stb_state);
		
		if (shbf >= 5){
			printf(" %s %d yeep\n", state[stbs.state], stbs.ctr );
			shbf = 0;
			shbf_done = 1;
		}

		//clk_val = bcm2835_gpio_lev( CLK );
		//dio_val = bcm2835_gpio_lev( DIO );
	}
	
	bcm2835_close();
	return 0;
}
