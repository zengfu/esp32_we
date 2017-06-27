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
#include "wm8978.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_event_loop.h"
#include "hal_i2c.h"
#include "hal_i2s.h"
#include "hal_eth.h"
#include "esp_log.h"



void app_main()
{
    
    esp_event_loop_init(NULL, NULL);
    char* samples_data = malloc(1024);
    hal_i2c_init(0,5,17);
    esp_err_t err;
    int cnt = 0;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5
    };
    sdmmc_card_t* card;
    err = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (err != ESP_OK) {
        if (err == ESP_FAIL) {
            printf("Failed to mount filesystem. If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            printf("Failed to initialize the card (%d). Make sure SD card lines have pull-up resistors in place.", err);
        }
        return;
    }
    sdmmc_card_print_info(stdout, card);
    
    
    
    hal_i2s_init(0,48000,16,2);
    WM8978_Init();
    WM8978_ADDA_Cfg(1,1); 
    WM8978_Input_Cfg(1,0,0);     
    WM8978_Output_Cfg(1,0); 
    WM8978_MIC_Gain(46);
    WM8978_SPKvol_Set(50);
    WM8978_EQ_3D_Dir(1);
    WM8978_EQ1_Set(3,24);
    WM8978_EQ2_Set(3,24);
    WM8978_EQ3_Set(3,24);
    WM8978_EQ4_Set(3,24);
    WM8978_EQ5_Set(3,24);
    // wm8978_init();
    // wm8978_speaker_init();
    // wm8978_dac_volume(220);
    // wm8978_adc_volume(200);
    // wm8978_mic_init();
    // wm8978_write_dump();

    //test
    //err=hal_eht_init();
    int wlen;
    while(1) {
        //vTaskDelay(1000 / portTICK_RATE_MS);
        //datalen= wav_head.wSampleLength;
       
            //
            aplay("/sdcard/test.wav");
            printf("%s\n", "ok!");
            // if(rlen!=100){
            //     printf("read file Failed");
            //     return;
            // }
            wlen=hal_i2s_read(0,samples_data,1024,1000);
            //printf("wlen:%d\n",wlen );
            hal_i2s_write(0,samples_data,1024,1000);
            //printf("%d\n", cnt++);
            // for(int i=0;i<1024;i++){
            //     if(samples_data[i]!=0)
            //         printf("%d\n",samples_data[i] );
            // }
            
        
        //fseek(f,sizeof(wav_head),SEEK_SET);
        // err=hal_i2c_master_mem_write(0,0x1a,0x02,&data,1);
        // if(err!=ESP_OK)
        //     printf("write faliled:%d\n",err);
    }
}

