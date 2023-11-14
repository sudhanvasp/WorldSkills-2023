#include <stdio.h>
#include <stdint.h>
#include "libschsat.h"
#define LSS_OK 0 
#define LSS_ERROR 1 
#define LSS_BREAK 2
 
#include <math.h>
const int time_step = 1;
const uint16_t mag_num = 1;
 
int mag_calibrated(int16_t *magx, int16_t *magy, int16_t *magz ){
 
	float magx_cal;
	magx_cal = 1.06*(*magx + -7.49) + -0.01*(*magy + -23.59) + 0.07*(*magz + -108.24);
	float magy_cal;
	magy_cal = -0.01*(*magx + -7.49) + 1.11*(*magy + -23.59) + 0.09*(*magz + -108.24);
	float magz_cal;
	magz_cal = 0.07*(*magx + -7.49) + 0.09*(*magy + -23.59) + 1.00*(*magz + -108.24);
	*magx = (int16_t) magx_cal;
	*magy = (int16_t) magy_cal;
	*magz = (int16_t) magz_cal;
 
	return 0;
}
 
void initialize_all(){
	printf("Enable magnetometer %d\n", mag_num);
	magnetometer_turn_on(mag_num);
	Sleep(1);
}
 
void switch_off_all(){
	printf("Disable magnetometer %d\n", mag_num);
	magnetometer_turn_off(mag_num);
}
 
int control(){
	initialize_all();
	int mag_state = 0;
	int16_t mgx_cal=0;
	int16_t mgy_cal=0;
	int16_t mgz_cal=0;
	int16_t *magx_cal = &mgx_cal;
	int16_t *magy_cal = &mgy_cal;
	int16_t *magz_cal = &mgz_cal;
	int i;
 
	for ( i= 0; i < 60; i++){
		mag_state = magnetometer_request_raw(mag_num, magx_cal,magy_cal,magz_cal);
		float mag_alpha;
		if (!mag_state){
			mag_calibrated(magx_cal,magy_cal,magz_cal);
			mag_alpha = atan2(mgy_cal, mgx_cal)/ M_PI *180;
			printf("mag_alpha atan2 = %f\n", mag_alpha);
		}
		else if (mag_state == 1){
			printf("Fail because of access error, check the connection");
		}
		else if (mag_state == 2){
			printf("Fail because of interface error, check your code");
		}
		Sleep(time_step);
	}
	switch_off_all();
	return 0;
}