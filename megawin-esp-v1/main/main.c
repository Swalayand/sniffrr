#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include <unistd.h>
#include <string.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"


#define STB_GPIO 17
#define CLK_GPIO 18 
#define DIO_GPIO 4 
#define GPIO_INPUT_PIN_SEL  (1ULL<<STB_GPIO | 1ULL<<CLK_GPIO | 1ULL<<DIO_GPIO )

#define LENGTH 100
#define FALLING 2
#define RISING  1

static const char *TAG = "megawin";

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

int car = 0;

int shbf_done = 0;
char res[54];

/*
void printEvent(void *arg){	
  int c = 0;
	for (int i = 0; i < 54; i++){
		if (ar[17] != 138 && ar[18] != 3 && ar[35] != 138 && ar[36] != 3) break;
		for (int j = 0; j < 20; j++){
			if ((ar[i] == full_digits[j] && ar[13] == 2) || (ar[i] == full_digits[j] && ar[13] == 3)) {
                //res += j%10;
        sprintf(res+(c++), "%d", j%10);
      }
    }
	}
	if (c > 5) printf("%s \n", res);
	//if (ar[17] == 138 || ar[5] >= 115 ) {printf("\n"); fflush( stdout );}
}
*/

void printEvent(void *arg){
	int c = 0;
	while(1){
		sleep(0.1);
		if (car < 54) continue;
		car = 0;
		if (ar[17] != 138 || ar[5] < 115 ) continue;

		for (int i = 0; i < 54; i++){	
			for (int j = 0; j < 20; j++)
				if ((ar[i] == full_digits[j] && ar[13] == 2) || (ar[i] == full_digits[j] && ar[13] == 3)) sprintf(res+(c++), "%d", j%10);
			//if (i % 18 == 0) printf("\t");
		}
		//printf("\n");
		c = 0;
		printf("%s\n", res);
	}

}

void eventCount(void *param){
	uint8_t stb_dat, stb_state, clk_dat, clk_state, dio_dat, ctr_dio = 0, value = 0;
	while(1){
		stb_dat = (GPIO.in >> STB_GPIO ) & 0x1;
		clk_dat = (GPIO.in >> CLK_GPIO ) & 0x1;
		stb_state = setState(stb_dat, g_s_prev, &stb_pin, &g_s_prev, &stb_pin_prev);
		clk_state = setState(clk_dat, g_c_prev, &clk_pin, &g_c_prev, &clk_pin_prev);
		
		if (stb_pin_prev.ctr > 10000 && stb_pin.state == FALLING) shbf_done = 1;			
		if (shbf_done == 1 && clk_pin.state == RISING){
			dio_dat = (GPIO.in >> DIO_GPIO ) & 0x1;
			if (ctr_dio > 7){
				ar[car++] = value;
				ctr_dio = 0; value = 0; 
			}
			value = value | (dio_dat << ctr_dio++);
			if ( (car+1) % 54 == 0 && car != 0 ) {
          shbf_done = 0;
      }
		}
	}
}

static void configure(){
		gpio_config_t io_conf;
		io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
		
    ESP_ERROR_CHECK(ret);
    configure();
    xTaskCreate(eventCount, "eventCount", 4096, NULL, 5, NULL);
		xTaskCreate(printEvent, "printEvent", 4096, NULL, 5, NULL);
}
