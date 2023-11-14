#include <stdio.h>
#include <stdint.h>
#include "libschsat.h"
#define LSS_OK 0 
#define LSS_ERROR 1 
#define LSS_BREAK 2
 
 
#include <math.h>
void control(void){
	int time_step = 1;
	uint16_t mag_num = 1;
	printf("Enable magnetometer %d\n", mag_num);
	magnetometer_turn_on(mag_num);
	Sleep(1);
	int mag_state = 0;
 
	int16_t p_dataX;
	int16_t p_dataY;
	int16_t p_dataZ;
	int16_t *pRAW_dataX = &p_dataX;
	int16_t *pRAW_dataY = &p_dataY;
	int16_t *pRAW_dataZ = &p_dataZ;
	int i;
	for (i = 0; i < 60; i++){
		mag_state = magnetometer_request_raw(mag_num, pRAW_dataX, pRAW_dataY, pRAW_dataZ);
		float mag_alpha;
		if (!mag_state){
			mag_alpha = atan2(p_dataY, p_dataX)/M_PI*180;
			printf("mag_alpha atan2 = %f\n", mag_alpha);
		}
		else if (mag_state == 1){
			printf("Fail because of access error, check the connection\n");
		}
		else if (mag_state == 2){
			printf("Fail because of interface error, check your code\n");
		}
		Sleep(time_step);
	}
	printf("Disable magnetometer %d\n", mag_num);
	magnetometer_turn_off(mag_num);
}