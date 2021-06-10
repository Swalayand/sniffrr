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

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include <lwip/netdb.h>

#define STB_GPIO 17
#define CLK_GPIO 18 
#define DIO_GPIO 4 
#define GPIO_INPUT_PIN_SEL  ( 1ULL<<STB_GPIO | 1ULL<<CLK_GPIO | 1ULL<<DIO_GPIO )

#define LENGTH 100
#define FALLING 2
#define RISING  1

//static const char *TAG = "megawin";

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


void printEvent(void *arg){
    int c = 0;
	while(1){
		sleep(0.2);
		if (car < 54) continue;
		car = 0;
		if (ar[17] != 138 || ar[5] < 115 ) continue;

		for (int i = 0; i < 54; i++){	
			for (int j = 0; j < 20; j++)
				if ((ar[i] == full_digits[j] && ar[13] == 2) || (ar[i] == full_digits[j] && ar[13] == 3)) sprintf(res+(c++), "%d", j%10);
		}
        printf("%s\n", res);
        c = 0;
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
			if ( (car+1) % 54 == 0 && car != 0 ) shbf_done = 0;
			
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

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      "CHART - Catat HARga Timbangan"//CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

#define PORT                        80 // 3333//CONFIG_EXAMPLE_PORT
#define KEEPALIVE_IDLE              5//CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          5//CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             3//CONFIG_EXAMPLE_KEEPALIVE_COUNT

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        //ESP_LOGI(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        //ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

static void do_retransmit(const int sock)
{
    int len;
    char rx_buffer[1024];
    char tx_buffer[1024] = "HTTP 200 OK\r\n\r\n";
    strcat(tx_buffer, res);
    len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
    send(sock, tx_buffer, strlen(tx_buffer), 0);

}

static void tcp_server_task(void *pvParameters)
{
    xTaskCreate(eventCount, "counting_event", 4096, NULL, 5, NULL);
    xTaskCreate(printEvent, "preparing_for_send", 4096, NULL, 5, NULL);

    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }
#ifdef CONFIG_EXAMPLE_IPV6
    else if (addr_family == AF_INET6) {
        struct sockaddr_in6 *dest_addr_ip6 = (struct sockaddr_in6 *)&dest_addr;
        bzero(&dest_addr_ip6->sin6_addr.un, sizeof(dest_addr_ip6->sin6_addr.un));
        dest_addr_ip6->sin6_family = AF_INET6;
        dest_addr_ip6->sin6_port = htons(PORT);
        ip_protocol = IPPROTO_IPV6;
    }
#endif

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        //ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
#endif

    //ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        //ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        //ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    //ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        //ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {

        //ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            //ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keep alive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
#ifdef CONFIG_EXAMPLE_IPV6
        else if (source_addr.ss_family == PF_INET6) {
            inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
#endif
        //ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    //ESP_LOGI(TAG, "Wifi finished. SSID:%s password:%s channel:%d", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
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
    
		//ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    
    wifi_init_softap();
    
//#ifdef CONFIG_EXAMPLE_IPV4
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);
    
//#endif
//#ifdef CONFIG_EXAMPLE_IPV6
//    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET6, 5, NULL);
//#endif
}
