/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "wm8978.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include <sys/socket.h>

#include "eth.h"
#include "event.h"
#include "tcp.h"
#include "telnet.h"

#define GPIO_OUTPUT_IO_0    16
#define GPIO_OUTPUT_PIN_SEL  ((1<<GPIO_OUTPUT_IO_0))

#define TAG "main:"
void app_main()
{
    event_engine_init();
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    tcpip_adapter_init();
    eth_init();
    //do{
        gpio_set_level(GPIO_OUTPUT_IO_0, 0);
        xEventGroupWaitBits(eth_event_group,ETH_CONNECTED_BIT,pdTRUE,pdTRUE,portMAX_DELAY);
        ESP_LOGI(TAG,"the eth link up");
        xEventGroupWaitBits(eth_event_group,ETH_GOTIP_BIT,pdTRUE,pdTRUE,portMAX_DELAY);
        //esp_err_t tcpip_adapter_get_ip_info(tcpip_adapter_if_t tcpip_if, tcpip_adapter_ip_info_t *ip_info);
        gpio_set_level(GPIO_OUTPUT_IO_0, 1);
        tcpip_adapter_ip_info_t ip;
        memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
        if (tcpip_adapter_get_ip_info(ESP_IF_ETH, &ip) == 0) {
            ESP_LOGI(TAG, "~~~~~~~~~~~");
            ESP_LOGI(TAG, "ETHIP:"IPSTR, IP2STR(&ip.ip));
            ESP_LOGI(TAG, "ETHPMASK:"IPSTR, IP2STR(&ip.netmask));
            ESP_LOGI(TAG, "ETHPGW:"IPSTR, IP2STR(&ip.gw));
            ESP_LOGI(TAG, "~~~~~~~~~~~");
        }
        //xEventGroupWaitBits(eth_event_group,ETH_DISCONNECTED_BIT,pdTRUE,pdTRUE,portMAX_DELAY);
    //}while(1);
    //if(create_tcp_server(8080)!=ESP_OK){
      //  return;
    //}
    xTaskCreate(vTelnetTask, "telnet_task", 2048, NULL, (tskIDLE_PRIORITY + 2), NULL);
    //char databuff[100]={0};
    //int len=0;
    while(1){
       vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

