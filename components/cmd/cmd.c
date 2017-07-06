#include "cmd.h"
#include "FreeRTOS_CLI.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "string.h"
#include "stdio.h"
#include "mqtt.h"


#define max_cmd_para 8
#define max_cmd_para_len 32

static BaseType_t prvTaskStatsCommand( char *pcWriteBuffer,
                                          size_t xWriteBufferLen,
                                          const char *pcCommandString );
static BaseType_t prvMqttSubCommand(     char *pcWriteBuffer,
                                                size_t xWriteBufferLen, 
                                                const char *pcCommandString );
static BaseType_t prvMqttPubCommand(     char *pcWriteBuffer,
                                                size_t xWriteBufferLen, 
                                                const char *pcCommandString );


static CLI_Command_Definition_t xTaskListCommand =
{
	"listtask",
	"\r\nlisttask:\r\n  lists all freerots's tasks\r\n",
	prvTaskStatsCommand,
	0
};
static CLI_Command_Definition_t MqttSubCommand =
{
  "mqtt_sub",
  "\r\nmqtt_sub:\r\n  subscribe a topc\r\n",
  prvMqttSubCommand,
  -1
};
static CLI_Command_Definition_t MqttPubCommand =
{
  "mqtt_pub",
  "\r\nmqtt_pub:\r\n publish a topic and payload\r\n",
  prvMqttPubCommand,
  -1
};

void CmdRegister(){
	FreeRTOS_CLIRegisterCommand(&xTaskListCommand);
  FreeRTOS_CLIRegisterCommand(&MqttSubCommand);
  FreeRTOS_CLIRegisterCommand(&MqttPubCommand);
}


extern mqtt_client *client;

static BaseType_t prvMqttPubCommand(     char* pcWriteBuffer,
                                                size_t xWriteBufferLen, 
                                                const char *pcCommandString )
{
    char *pcParameter;
    BaseType_t lParameterStringLength;

    /* Note that the use of the static parameter means this function is not reentrant. */
    uint32_t argc = 0;
    char argv[max_cmd_para][max_cmd_para_len]={{0}};

        /* lParameter is not 0, so holds the number of the parameter that should
        be returned.  Obtain the complete parameter string. */
    for(argc=0;argc<max_cmd_para;argc++){

        pcParameter = ( char * ) FreeRTOS_CLIGetParameter
                                (
                                    /* The command string itself. */
                                    pcCommandString,
                                    /* Return the next parameter. */
                                    argc+1,
                                    /* Store the parameter string length. */
                                    &lParameterStringLength
                                );

        if( pcParameter != NULL ){
            strncat(argv[argc],pcParameter,lParameterStringLength);
        }
        else{
          break;
        }
    }
    if(argc==max_cmd_para||argc==0){
      sprintf(pcWriteBuffer,"  Error: You must specify a topic and a payload\r\n  Use 'mqtt_pub --help' to see usage\r\n");
      return pdFALSE;
    }
    if(strcmp(argv[0],"--help")==0){
      sprintf(pcWriteBuffer,
          "  Usage: mqtt_pub -t topic -p palyload\r\n");
      return pdFALSE;
    }
    if(argc==4){
      if(strcmp(argv[0],"-t")==0&&strcmp(argv[2],"-p")==0){
          mqtt_publish(client, argv[1], argv[3],strlen(argv[3]),0,0);
          sprintf(pcWriteBuffer,"  publish success\r\n");
          return pdFALSE;
      }else if(strcmp(argv[2],"-t")==0&&strcmp(argv[0],"-p")==0){
          mqtt_publish(client, argv[3], argv[1],strlen(argv[3]),0,0);
          sprintf(pcWriteBuffer,"  publish success\r\n");
          return pdFALSE;
      }
    }
    sprintf(pcWriteBuffer,"  parameter is error\r\n");
    return pdFALSE;


    
}

static BaseType_t prvMqttSubCommand(     char* pcWriteBuffer,
                                                size_t xWriteBufferLen, 
                                                const char *pcCommandString )
{
    char *pcParameter;
    BaseType_t lParameterStringLength;

    /* Note that the use of the static parameter means this function is not reentrant. */
    uint32_t argc = 0;
    char argv[max_cmd_para][max_cmd_para_len]={{0}};

        /* lParameter is not 0, so holds the number of the parameter that should
        be returned.  Obtain the complete parameter string. */
    for(argc=0;argc<max_cmd_para;argc++){

        pcParameter = ( char * ) FreeRTOS_CLIGetParameter
                                (
                                    /* The command string itself. */
                                    pcCommandString,
                                    /* Return the next parameter. */
                                    argc+1,
                                    /* Store the parameter string length. */
                                    &lParameterStringLength
                                );

        if( pcParameter != NULL ){
            strncat(argv[argc],pcParameter,lParameterStringLength);
        }
        else{
          break;
        }
    }
    if(argc==max_cmd_para||argc==0){
      sprintf(pcWriteBuffer,"  Error: You must specify a topic to subscribe to.\r\n  Use 'mqtt_sub --help' to see usage\r\n");
      return pdFALSE;
    }
    if(strcmp(argv[0],"--help")==0){
      sprintf(pcWriteBuffer,
          "  Usage: mqtt_sub -t topic [-q qos]\r\n");
      return pdFALSE;
    }
   
    if(argc==2){
      mqtt_subscribe(client, argv[1], 0);
      sprintf(pcWriteBuffer,"  subscribe success\r\n");
      return pdFALSE;
    }
    if(argc==4){
      int qos;
      if(strcmp(argv[0],"-t")==0&&strcmp(argv[2],"-q")==0){
        qos=atoi(argv[3]);
        if(qos<3&&qos>=0){
          mqtt_subscribe(client, argv[1], qos);
          sprintf(pcWriteBuffer,"  subscribe success\r\n");
          return pdFALSE;
        }
      }else if(strcmp(argv[2],"-t")==0&&strcmp(argv[0],"-q")==0){
        qos=atoi(argv[1]);
        if(qos<3&&qos>=0){
          mqtt_subscribe(client, argv[3], qos);
          sprintf(pcWriteBuffer,"  subscribe success\r\n");
          return pdFALSE;
        }
      }

    }
    sprintf(pcWriteBuffer,"  parameter is error\r\n");
    return pdFALSE;


    
}


/* This function implements the behaviour of a command, so must have the correct
prototype. */
static BaseType_t prvTaskStatsCommand( char *pcWriteBuffer,
                                          size_t xWriteBufferLen,
                                          const char *pcCommandString )
{
    /* For simplicity, this function assumes the output buffer is large enough
    to hold all the text generated by executing the vTaskList() API function,
    so the xWriteBufferLen parameter is not used. */
    ( void ) xWriteBufferLen;

    /* pcWriteBuffer is used directly as the vTaskList() parameter, so the table
    generated by executing vTaskList() is written directly into the output
    buffer. */
    vTaskList( pcWriteBuffer );

    /* The entire table was written directly to the output buffer.  Execution
    of this command is complete, so return pdFALSE. */
    return pdFALSE;
}