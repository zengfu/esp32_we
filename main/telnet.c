/* Standard includes. */
#include "string.h"

/* lwIP core includes */
#include "lwip/opt.h"
#include "lwip/sockets.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/* Utils includes. */
#include "FreeRTOS_CLI.h"

#include "event.h"
#include "cmd.h"

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE	34
#define cmdMAX_OUTPUT_SIZE  1024
static char outbuf[cmdMAX_OUTPUT_SIZE]={0};
static char path[32]={0};
void vTelnetTask( void *pvParameters ){
	int32_t lSocket, lClientFd, lBytes, lAddrLen = sizeof( struct sockaddr_in );
	struct sockaddr_in sLocalAddr;
	struct sockaddr_in client_addr;
	const char* pcWelcomeMessage = "WhyEngineer command server - connection accepted.\r\nType Help to view a list of registered commands.\r\n";
	static char cInputString[ cmdMAX_INPUT_SIZE ] = { 0 }, cLastInputString[ cmdMAX_INPUT_SIZE ] = { 0 };
	portBASE_TYPE xReturned;
	
	(void) pvParameters;
	CmdRegister();
	lSocket = lwip_socket( AF_INET, SOCK_STREAM, 0 );
	if( lSocket >= 0 ){
		/* Obtain the address of the output buffer.  Note there is no mutual
		exclusion on this buffer as it is assumed only one command console
		interface will be used at any one time. */

		memset((char *)&sLocalAddr, 0, sizeof(sLocalAddr));
		sLocalAddr.sin_family = AF_INET;
		sLocalAddr.sin_len = sizeof(sLocalAddr);
		sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		sLocalAddr.sin_port = ntohs( ( ( uint16_t ) 23 ) );

		if( lwip_bind( lSocket, ( struct sockaddr *) &sLocalAddr, sizeof( sLocalAddr ) ) < 0 ) {
			lwip_close( lSocket );
			vTaskDelete( NULL );
		}

		if( lwip_listen( lSocket, 20 ) != 0 ){
		
			lwip_close( lSocket );
			vTaskDelete( NULL );
		}

		/* Ensure the input string starts clear. */
		cInputString[ 0 ] = 0;
		cLastInputString[ 0 ] = 0;
		uint32_t length;
		sprintf(path,"root@we:/#");
		for(;;){
			lClientFd = lwip_accept( lSocket, ( struct sockaddr * ) &client_addr, ( u32_t * ) &lAddrLen );
			if(lClientFd>0L){
				strcat(outbuf,pcWelcomeMessage);
				strcat(outbuf,path);
				lwip_send( lClientFd, outbuf, strlen(outbuf), 0 );
				do{
					lBytes = lwip_recv( lClientFd, cInputString, sizeof( cInputString ), 0 );
					for(int i=lBytes-1;i>=0;i--){
						if (cInputString[i]=='\n'||cInputString[i]=='\r'){
							cInputString[i]='\0';
						}else{
							break;
						}
					}
					if(strlen(cInputString)==0){
						lwip_send( lClientFd,path,strlen(path), 0 );
						continue;
					}
					printf("%s\n",cInputString );
					do{
						outbuf[0] = 0x00;
						xReturned=FreeRTOS_CLIProcessCommand(cInputString,outbuf,cmdMAX_OUTPUT_SIZE);
						length=strlen(outbuf);
						sprintf(outbuf+length,"%s","---------\r\n");
						lwip_send( lClientFd, outbuf, strlen(outbuf), 0 );
					}
					while(xReturned!=pdFALSE);
				lwip_send( lClientFd,path,strlen(path), 0 );
				}while(lBytes>0L);
			}
			lwip_close( lClientFd );
		}
	}
	

}