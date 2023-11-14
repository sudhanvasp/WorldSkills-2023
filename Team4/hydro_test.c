#include "libschsat.h"
 
int control ( ) { 
	uint16_t num =  1 ;   	// Number of the Angular Rate Sensor 
	int16_t hyro_result [ ]  =  { 0 ,  0 ,  0 ,  0 } ; 
	printf ( "Enable angular velocity sensor No. %d" , num ) ;  
	hyro_turn_on ( num ) ;  						// Turn on the DOS 
	Sleep ( 1 ) ; 							//Wait for 1 second to turn on 
	printf ( " \n Get RAW data from angular velocity sensor \n " ) ;
 
	int i ; 
	for  ( i =  0 ; i <  10 ; i ++ )  {  		//Read the readings 10 times 
	hyro_result [ 0 ]  = hyro_request_raw ( num ,& hyro_result [ 1 ] ,& hyro_result [ 2 ] ,& hyro_result [ 3 ] ) ;
 
	if  ( ! hyro_result [ 0 ] ) 
		{ 
			printf ( "state: %d" , i ) ; 
			printf ( " x_raw = %d " , hyro_result [ 1 ] ) ; 
			printf ( " y_raw = %d " , hyro_result [ 2 ] ) ; 
			printf ( "z_raw = %d \n " , hyro_result [ 3 ] ) ;
 
		} 
		else  if  ( hyro_result [ 0 ]  ==  1 ) 
		{ 
			printf ( "Fail because of access error, check the connection" ) ; 
		} 
		else  if  ( hyro_result [ 0 ]  ==  2 ) 
		{ 
			printf ( "Fail because of interface error, check your code" ) ; 
		} 
		Sleep ( 1 ) ;
 
	} 	
	printf ( " \n Disable angular velocity sensor № %d \n " , num ) ; 
	hyro_turn_off ( num ) ; 	
	return  0 ; 
}
main(){
    control();}