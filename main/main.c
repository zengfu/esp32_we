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
#include "wm8979.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_event_loop.h"
#include "hal_i2c.h"
#include "hal_i2s.h"
#include "hal_eth.h"
#include "esp_log.h"

typedef struct 
{
    char rld[4];    //riff 标志符号
    int  rLen;      //
    char wld[4];    //格式类型（wave）
    char fld[4];    //"fmt"
 
    int fLen;   //sizeof(wave format matex)
 
    short wFormatTag;   //编码格式
    short wChannels;    //声道数
    int   nSamplesPersec;  //采样频率
    int   nAvgBitsPerSample;//WAVE文件采样大小
    short wBlockAlign; //块对齐
    short wBitsPerSample;   //WAVE文件采样大小

    char dld[4];        //”data“
    int  wSampleLength; //音频数据的大小
 }WAV_HEADER;


void app_main()
{
    WAV_HEADER wav_head;
    esp_event_loop_init(NULL, NULL);
    char* samples_data = malloc(100);
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
    FILE* f = fopen("/sdcard/test.wav", "r");
    if (f == NULL) {
        printf("Failed to open file for writing");
        return;
    }
    //fprintf(f, "Hello %s!\n", card->cid.name);
    int rlen=fread(&wav_head,1,sizeof(wav_head),f);
    if(rlen!=sizeof(wav_head)){
        printf("%s\n","read faliled");
        return;
    }
    int channels = wav_head.wChannels;
    int frequency = wav_head.nSamplesPersec;
    int bit = wav_head.wBitsPerSample;
    int datalen;
    printf("channels:%d,frequency:%d,bit:%d\n",channels,frequency,bit);
    hal_i2s_init(0,48000,16,2);
    wm8979_init();
    //test
    //err=hal_eht_init();
    int wlen;
    while(1) {
        printf("cnt: %d\n", cnt++);
        //vTaskDelay(1000 / portTICK_RATE_MS);
        datalen= wav_head.wSampleLength;
        while(datalen){
            rlen=fread(samples_data,1,100,f);

            if(rlen!=100){
                printf("read file Failed");
                return;
            }
            wlen=hal_i2s_write(0,samples_data,100,1000);
            if(wlen!=100){
                printf("i2s write faliled");
                return;
            }
            datalen-=rlen;
        }
        fseek(f,sizeof(wav_head),SEEK_SET);
        // err=hal_i2c_master_mem_write(0,0x1a,0x02,&data,1);
        // if(err!=ESP_OK)
        //     printf("write faliled:%d\n",err);
    }
}

