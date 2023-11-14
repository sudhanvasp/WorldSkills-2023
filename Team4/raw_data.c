#include <stdio.h>
#include <stdint.h>
#include "libschsat.h"
#define LSS_OK 0 
#define LSS_ERROR 1 
#define LSS_BREAK 2
 
 
void control(void){ //Program’s main function
	int16_t mgn_result[] = {0, 0, 0, 0}; //Initialize mgn_result
	uint16_t num = 1;
	magnetometer_turn_on(num); //Turn magnetometer on 
	Sleep(1);
	int i;
	for (i = 0; i < 1000; i++) //Perform 500 readings
	{
		mgn_result[0] = magnetometer_request_raw(num, &mgn_result[1],&mgn_result[2],&mgn_result[3]);
		if(!mgn_result[0]){ //If sensor has not returned error message
			printf("%d, %d, %d\n", mgn_result[1], mgn_result[2], mgn_result[3]);
		}
		Sleep(0.5); //Delay five-hundredths of the second
	}
	magnetometer_turn_off(num); 
}