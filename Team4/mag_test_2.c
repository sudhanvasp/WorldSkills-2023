#include <stdio.h>
#include <stdint.h>
#include "libschsat.h"
#define LSS_OK 0 
#define LSS_ERROR 1 
#define LSS_BREAK 2
 
void control(void) // Program’s main function which is calling all other functions
{
	int16_t mgn_result[] = {0, 0, 0, 0}; //Initialize mgn_result
	uint16_t num = 1; //Magnetometer number
	printf("Enable magnetometer № %d", num);
	magnetometer_turn_on(num); //Magnetometer turn on
	Sleep(1);
	printf("Get RAW data from magnetometer");
	int i;
	for (i = 0; i < 10; i++)
	{
		mgn_result[0] = magnetometer_request_raw(num, &mgn_result[1], &mgn_result[2], &mgn_result[3]);
		if (!mgn_result[0]) //if error message
		{
			printf("state: %d\n", i);
			printf(" x_raw = %d\n", mgn_result[1]);
			printf(" y_raw = %d\n", mgn_result[2]);
			printf(" z_raw = %d\n", mgn_result[3]);
		}
		else if (mgn_result[0] == 1)
		{
			printf("Fail because of access error, check the connection");
		}
		else if (mgn_result[0] == 2)
		{
			printf("Fail because of interface error, check your code");
		}
		Sleep(1);
	}
	printf("Disable magnetometer №%d", num);
	magnetometer_turn_off(num); //Magnetometer turn off
 
 
}