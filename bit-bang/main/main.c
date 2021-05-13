
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"


/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.

   /Users/e/Library/Arduino15/packages/esp32/tools/esptool_py/2.6.1/esptool --chip esp32 --port /dev/cu.usbserial-0001 --baud 115200  /Users/e/Library/Arduino15/packages/esp32/hardware/esp32/1.0.4/tools/partitions/boot_app0.bin 0x1000 /Users/e/Library/Arduino15/packages/esp32/hardware/esp32/1.0.4/tools/sdk/bin/bootloader_dio_80m.bin 0x10000 /var/folders/q9/l8chc4d52lgfxxh7b_0k0nw00000gp/T/arduino_build_630350/TM1638_demo.ino.bin 0x8000 /var/folders/q9/l8chc4d52lgfxxh7b_0k0nw00000gp/T/arduino_build_630350/TM1638_demo.ino.partitions.bin 


   /Users/e/Library/Arduino15/packages/esp32/tools/esptool_py/2.6.1/esptool --chip esp32 --port /dev/cu.usbserial-0001 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xe000 /Users/e/Library/Arduino15/packages/esp32/hardware/esp32/1.0.4/tools/partitions/boot_app0.bin 0x1000 /Users/e/Library/Arduino15/packages/esp32/hardware/esp32/1.0.4/tools/sdk/bin/bootloader_dio_80m.bin 0x10000 /var/folders/q9/l8chc4d52lgfxxh7b_0k0nw00000gp/T/arduino_build_630350/TM1638_demo.ino.bin 0x8000 /var/folders/q9/l8chc4d52lgfxxh7b_0k0nw00000gp/T/arduino_build_630350/TM1638_demo.ino.partitions.bin 
*/
#define BLINK_GPIO 17 // CONFIG_BLINK_GPIO

#define CLOCK_NORMAL 0
#define CLOCK_INVERT 1

#define LSBFIRST 0
#define HIGH 1
#define LOW 0

#define CLOCK_TYPE CLOCK_INVERT
#define CLOCK_DELAY_US 1

const int strobe_pin =  4; // STB
const int clock_pin  = 16; // CLK
const int data_pin   = 17; // DIO

#define COUNTING_MODE 0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "freertos/task.h"

#include "esp_timer.h"
#include "esp_intr_alloc.h"
//#include "pca9555.h"

#include "driver/gpio.h"

#define RTOS_TASK_PRIORITY_NORMAL (5)

#define GPIO_STB     4
#define GPIO_CLK     16
#define GPIO_DIO     17

// https://github.com/espressif/arduino-esp32/issues/4067
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_STB | 1ULL<<GPIO_CLK | 1ULL<<GPIO_DIO )
#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle gpio_evt_queue = NULL;
static xQueueHandle gpio_clk_queue = NULL;
int ready = 0;
static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;  xQueueSendFromISR( gpio_evt_queue, &gpio_num, NULL );
}
static void IRAM_ATTR gpio_clk_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;  xQueueSendFromISR( gpio_clk_queue, &gpio_num, NULL );
}
static void i2c_isr_gpio_task(void* arg) {
    uint32_t io_num;
    for(;;) {
        if( xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            //printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));  
			ready = 1;     
		}
    }
}
static void i2c_w_task(void* arg) {
    uint32_t io_num;
	uint8_t val;
	uint8_t i = 0;
    for(;;) {
		val = 0;
        if( xQueueReceive(gpio_clk_queue, &io_num, portMAX_DELAY)  ) 
		{
            val |= gpio_get_level(io_num) << i; i++;
			if( i > 7 ) {
              printf("%d %d\n", io_num, val );  
			  val = 0; i=0; 
			}    
		}
    }
}
int isr_io_config() {
	gpio_config_t io_conf;
    io_conf.intr_type    = GPIO_PIN_INTR_DISABLE; //disable interrupt
    io_conf.mode         = GPIO_MODE_INPUT; //set as output mode
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL; //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pull_down_en = 0; //disable pull-down mode
    io_conf.pull_up_en   = 1;  //disable pull-up mode
    gpio_config( &io_conf ); //configure GPIO with the given settings


/* // AKTIFKAN Interrupt
	gpio_set_intr_type(GPIO_STB, GPIO_INTR_NEGEDGE); // falling edge,  https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
	gpio_evt_queue = xQueueCreate( 100, sizeof(uint32_t) ); //install gpio isr service
    gpio_install_isr_service( ESP_INTR_FLAG_DEFAULT );
    gpio_isr_handler_add( GPIO_STB , gpio_isr_handler, (void*) GPIO_STB); //hook isr handler for specific gpio pin

	gpio_set_intr_type( GPIO_CLK, GPIO_INTR_POSEDGE ); // rising edge,  https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
	gpio_clk_queue = xQueueCreate( 100, sizeof(uint32_t) ); //install gpio isr service
    //gpio_install_isr_service( ESP_INTR_FLAG_DEFAULT );
    gpio_isr_handler_add( GPIO_CLK , gpio_clk_handler, (void*) GPIO_DIO ); //hook isr handler for specific gpio pin
	*/
	return 0;
}
void app_main2()
{
//	pca9555_init();
	isr_io_config();
	xTaskCreate( i2c_isr_gpio_task, "i2c_isr_gpio_task", 2048, NULL, 10, NULL);
	//xTaskCreate( i2c_w_task, "i2c_w_task", 2048, NULL, 10, NULL);
}

#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
static void init_sdcard() // tested on esp32 cam
{
  esp_err_t ret = ESP_FAIL;
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 3,
  };
  sdmmc_card_t *card;

  printf( "Mounting SD card...\n");
  ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

  if (ret == ESP_OK)
  {
    printf( "SD card mount successfully!\n");
  }
  else
  {
    printf( "Failed to mount SD card VFAT filesystem. Error: %s\n", esp_err_to_name(ret));
  }

    char *fn = "/sdcard/rec1.txt";
    FILE *file = fopen( fn , "w");
    if (file != NULL)
    {
      size_t err = fwrite( fn, 1, strlen(fn), file);
      printf( "File saved: %s\n", fn );
    }
    else
    {
      printf( "Could not open file %s\n", fn );
    }
    fclose(file);

}

typedef struct  {
	uint8_t state;
	int     ctr ;
} state_t;

#define AMAX 2000
state_t astb[ AMAX ], aclk[ AMAX ], adat[ AMAX ];
int cstb=0, cclk=0, cdat=0 ; //counter array stb

void read_bitbybit(void *pvParameter) {
	uint8_t clk, clk0=0, clks , dat , dat0=0, dats, stb, stb0=0 , stbs; // clks = clkStatus
	uint64_t timer_start , ts =0, tc, td, ctr=1;
	char *state[] = { "LOW", "RISING", "FALING", "HIGH", "DEF" };
	int i = 0;
	for(i=0;i<AMAX;i++) { 
		astb[i].state = 4; astb[i].ctr = 0;
		aclk[i].state = 4; aclk[i].ctr = 0;
		adat[i].state = 4; adat[i].ctr = 0; 
	}
	for(i=0;i<AMAX;i++) {
	  ts   = esp_timer_get_time();
	  stb  = (GPIO.in >> strobe_pin ) & 0x1; //gpio_get_level( strobe_pin ); // faster pin read https://pastebin.com/raw/AT9Z6hEJ
	  // implementasi sederhana deteksi RISING dan FALLING: 00 = LOW , 01=RISING, 10=FALING, 11=HIGH
	  stbs =  stb  | stb0 << 1; // 
	  if ( astb[ cstb ].state == stbs ) // 
	    astb[ cstb ].ctr++;
	  else {
	    astb[ cstb ].state = stbs;
		astb[ cstb ].ctr++;
	  }  
	  if ( stb != stb0 ) cstb++;  //
	  stb0 = stb;

	  //ets_delay_us( 1 );
	  tc   = esp_timer_get_time();
	  clk  = gpio_get_level( clock_pin ); // digitalRead
	  clks =  clk | clk0 << 1;
	  if ( aclk[ cclk ].state == clks ) // 
	    aclk[ cclk ].ctr++;
	  else {
	    aclk[ cclk ].state = clks;
		aclk[ cclk ].ctr++;
	  }  
	  if ( clk != clk0 ) cclk++;  //
	  clk0 = clk;
	  //ets_delay_us( 1 );

	  td   = esp_timer_get_time();
	  dat  = gpio_get_level( data_pin ); // 50ns https://www.reddit.com/r/esp32/comments/f529hf/results_comparing_the_speeds_of_different_gpio/
      dats =  dat | dat0 << 1;
	  if ( adat[ cdat ].state == dats ) // 
	    aclk[ cdat ].ctr++;
	  else {
	    aclk[ cdat ].state = dats;
		aclk[ cdat ].ctr++;
	  }  
	  if ( dat != dat0 ) cdat++;  //
      dat0 = dat;
	  //ets_delay_us( 1 );
	  timer_start = esp_timer_get_time();
	  //if( ctr % 1000 == 0 )
	   printf( "%d  %d  %d %6s  %6s  %6s %6lld %6lld %6lld \n", stb, clk, dat, state[stbs], state[clks], state[dats],  tc-ts, td-tc, timer_start-td )	 ;
	  ts =  timer_start;
	  ctr++;
	}
	
	for(i=0;i<AMAX;i++) { 
	   printf( "%4d %6s  %4d  %6s  %4d %6s  %4d \n", i,state[ astb[ i ].state ], astb[ i ].ctr, 
	     state[ aclk[ i ].state], aclk[ i ].ctr, 
		 state[ adat[ i ].state ], adat[ i ].ctr );
	   //printf( "%4d %6d  %4d  %6d  %4d %6d  %4d \n", i,  astb[ i ].state , astb[ i ].ctr,  aclk[ i ].state, aclk[ i ].ctr,  adat[ i ].state , adat[ i ].ctr );
	}


}

#include "soc/rtc_wdt.h"
#include "esp_int_wdt.h"
#include "esp_task_wdt.h"

void app_main(void) {
	//app_main2(  );
	// disable watchdog
  rtc_wdt_protect_off();
  rtc_wdt_disable();
  //disableCore0WDT();
  //disableLoopWDT();

  //init_sdcard();
	isr_io_config();
	/*
    gpio_reset_pin( clock_pin );    
	gpio_set_direction( clock_pin, GPIO_MODE_INPUT ); 
	//gpio_set_pull_mode(clock_pin, GPIO_PULLUP_ONLY);

    gpio_reset_pin( data_pin );     gpio_set_direction( data_pin, GPIO_MODE_INPUT); 
    gpio_reset_pin( strobe_pin );     gpio_set_direction( strobe_pin, GPIO_MODE_INPUT); 
*/

    BaseType_t xReturned;
    xReturned = xTaskCreate( &read_bitbybit, "read_bitbybit (name)", 2048, NULL, RTOS_TASK_PRIORITY_NORMAL, NULL);
    if (xReturned == pdPASS) {  printf( "OK Task has been created, and is running right now\n"); }
	read_bitbybit( NULL );
	while(1) {
        vTaskDelay(1000/portTICK_PERIOD_MS);
	}

}

