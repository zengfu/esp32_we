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
#include "hal_i2c.h"
#include "hal_i2s.h"
#include "wm8979.h"



void app_main()
{
    hal_i2c_init(0,5,17);
    uint8_t data=0;
    esp_err_t err;
    int cnt = 0;
    hal_i2s_init(0,48000,16,2);
    wm8979_init();
    while(1) {
        printf("cnt: %d\n", cnt++);
        vTaskDelay(1000 / portTICK_RATE_MS);
        // err=hal_i2c_master_mem_write(0,0x1a,0x02,&data,1);
        // if(err!=ESP_OK)
        //     printf("write faliled:%d\n",err);
    }
}

