/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "driver/spi_master.h"

#include "esp_http_client.h"

// Icludes of project
#include "http_request.h"
#include "wifi_connect.h" 
#include "spi_sensor.h"

static char msnSend[40];
static int32_t timeSend = 2000;
/* Sensor variables task */
static int16_t temperature = 0;
static uint16_t timePeriodo = 1000;

static const char *TAG = "___wifi_station___";

/* Task: SPI MAX6675 - Temperature Sensor Thermocouple K  */
void temp_task(void * pvParameters) {
  spi_device_handle_t spi = (spi_device_handle_t) pvParameters;
  while(true)
  {
    temperature = read_temp(spi);
    printf("SPI INT = %d temp=%f\n", temperature, temperature * 0.25);
    vTaskDelay(timePeriodo / portTICK_PERIOD_MS);
    //counter_t++;
  }
  vTaskDelete(NULL);
}

/* */
static void http_test_task(void *pvParameters)
{
	int counter = 0;
	while(counter<5){
		counter++;
		sprintf(msnSend,"{\"tempTk\":\"%d\"}",temperature);
		ESP_LOGI(TAG, "Contador: %d bytes: mensaje: %s", temperature, msnSend);
    vTaskDelay(timeSend / portTICK_PERIOD_MS);	
		
    http_post(msnSend);
    http_get();
  }
    ESP_LOGI(TAG, "Finish http example");
    vTaskDelete(NULL);
}

/* Root cert for howsmyssl.com, taken from howsmyssl_com_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/


void app_main(void)
{
    spi_device_handle_t spi;
    spi = spi_init();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
    ESP_LOGI(TAG, "Connected to AP, begin http example");

    xTaskCreate( &temp_task, "temperature_task", 4096, spi, 5, NULL );
    xTaskCreate( &http_test_task, "http_test_task", 1024*10, NULL, 5, NULL );
}
