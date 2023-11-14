#include <stdio.h> 
#include <stdint.h> 
#include "libschsat.h" 
#define LSS_OK 0 
#define LSS_ERROR 1 
#define LSS_BREAK 2
 
int control ( ) {  //Main function of the program 
	uint16_t sun_result [ ]  =  { 0 ,  0 ,  0 } ;  // initialize sun_result 
	uint16_t num =  1 ;  // sun sensor number 
	printf ( "Enable sun sensor №%d \n " , num ) ; 
	sun_sensor_turn_on ( num ) ;  //turn on the sensor	 
	Sleep ( 1 ) ;  //Wait for 1 second to turn on 
	printf ( "Get RAW data from sun sensor №%d \n " , num ) ; 
	int i ; 
	for  ( i =  0 ; i <  10 ; i ++ )  //read the readings 10 times 
	{ 
		sun_result [ 0 ]  = sun_sensor_request_raw ( num ,& sun_result [ 1 ] ,& sun_result [ 2 ] ) ; /*check how it works, it’s very strange that it works
		if it doesn't work, set sun_result[0] */ 
		if  ( ! sun_result [ 0 ] ) {  //if the sensor did not return an error message, 
			printf ( "state: %d raw = %d, %d \n " , i , sun_result [ 1 ] , sun_result [ 2 ] ) ; 
			}
 
		else  if  ( sun_result [ 0 ]  ==  1 )  {  //if the sensor returned error message 1 
			printf ( "Fail because of access error, check the connection \n " ) ; 
			} 
		else  if  ( sun_result [ 0 ]  ==  2 )  {  //if the sensor returned error message 2 
			printf ( "Fail because of interface error, check you code \n " ) ; 
			} 
		Sleep ( 1 ) ;  //readings are read once per second
 
		} 
	printf ( "Disable sun sensor №%d \n " , num ) ; 
	sun_sensor_turn_off ( num ) ;  //turn off the solar sensor 
	return  0 ; 
}