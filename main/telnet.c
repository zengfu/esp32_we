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

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE	34
#define cmdMAX_OUTPUT_SIZE  256
static char outbuf[cmdMAX_OUTPUT_SIZE]={0};
void vTelnetTask( void *pvParameters ){
	int32_t lSocket, lClientFd, lBytes, lAddrLen = sizeof( struct sockaddr_in );
	struct sockaddr_in sLocalAddr;
	struct sockaddr_in client_addr;
	const int8_t * const pcWelcomeMessage = ( const int8_t * ) "WhyEngineer command server - connection accepted.\r\nType Help to view a list of registered commands.\r\n\r\n>";
	static char cInputString[ cmdMAX_INPUT_SIZE ] = { 0 }, cLastInputString[ cmdMAX_INPUT_SIZE ] = { 0 };
	portBASE_TYPE xReturned;
	
	(void) pvParameters;

	lSocket = lwip_socket( AF_INET, SOCK_STREAM, 0 );

	if( lSocket >= 0 ){
		/* Obtain the address of the output buffer.  Note there is no mutual
		exclusion on this buffer as it is assumed only one command console
		interface will be used at any one time. */
		char * pcOutputString = FreeRTOS_CLIGetOutputBuffer();

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

		for(;;){
			lClientFd = lwip_accept( lSocket, ( struct sockaddr * ) &client_addr, ( u32_t * ) &lAddrLen );
			if(lClientFd>0L){
				lwip_send( lClientFd, pcWelcomeMessage, strlen( ( const char * ) pcWelcomeMessage ), 0 );
				do{
					lBytes = lwip_recv( lClientFd, cInputString, sizeof( cInputString ), 0 );
					printf("%s\n",cInputString );
					do{
						outbuf[ 0 ] = 0x00;
						xReturned=FreeRTOS_CLIProcessCommand(cInputString,outbuf,cmdMAX_OUTPUT_SIZE);
						lwip_send( lClientFd, outbuf, strlen(outbuf), 0 );
					}
					while(xReturned!=pdFALSE);

				}while(lBytes>0L);
			}
			lwip_close( lClientFd );
		}
	}
	

}