/* 
	DHT22 temperature sensor driver
*/

#ifndef DHT22_H_  
#define DHT22_H_

#define DHT_OK 0
#define DHT_CHECKSUM_ERROR -1
#define DHT_TIMEOUT_ERROR -2

// == function prototypes =======================================

void 	DHTsetgpio(int gpio);
void 	DHTerrorHandler(int response);
int 	DHTread();
float 	DHTgetHumidity();
float 	DHTgetTemperature();
int 	DHTgetSignalLevel( int usTimeOut, bool state );

#endif