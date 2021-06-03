#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"


#define GPIO_INPUT_IO_0     4 
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_0)

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
		configure();

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    int prev = 0, cnt = 0, ctr = 0, ctrl = 0;
		int i = 0;
    while(i++ < 10000000) {
				cnt = (GPIO.in >> GPIO_INPUT_IO_0 ) & 0x1; // cara lain dari gpio_get_level()
				if (cnt == 1) ctr++;
				else ctrl++; 
    }
		printf("\n%d %d\n", ctr, ctrl); fflush(stdout);
}
