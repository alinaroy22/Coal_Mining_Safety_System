/* 
	MQ7 Gas sensor driver
*/

#ifndef mq_7_H_  
#define mq_7_H_

#define MQ7_OK				 0
#define MQ7_CHECKSUM_ERROR	 -1
#define MQ7_TIMEOUT_ERROR	 -2

// == function prototypes =======================================

void 	MQ7setgpio(int gpio);
void 	MQ7errorHandler(int response);
int 	MQ7read();
float 	MQ7getConcentration();
int 	MQ7getSignalLevel( int usTimeOut, bool state );

#endif