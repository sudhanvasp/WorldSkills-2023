#include <stdio.h>
#include <stdint.h>
#include "libschsat.h"
#define LSS_OK 0 
#define LSS_ERROR 1 
#define LSS_BREAK 2
#include <math.h>
 
const int kd = 200; /* Differential feedback coefficient. If the satellite’s angular velocity is positive, than satellite must be spun clock-wise (i.e. the flywheel must rotated clock-wise)*/
 
const int kp = -50; /* Proportional feedback coefficient. If the current angle is more than target angle, than satellite must be spun counter-clock-wise (i.e. the flywheel must rotated counter-clock-wise).*/
const float time_step = 0.05; 			// working time-step
const uint16_t mtr_num = 1;			// flywheel number
const uint16_t hyr_num = 1; 		        // angular velocity sensor number
const uint16_t mag_num = 1;			// magnetometer number
// Maximum allowable flywheel speed, rpm
const int16_t mtr_max_speed = 5000;
int16_t mtr_new_speed;
 
int mag_calibrated(int16_t *magx, int16_t *magy, int16_t *magz ){
//mag_calibrated function will adjust the magnetometer’s readings considering the calibration factors
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
 
int motor_new_speed_PD(int mtr_speed, int omega, int omega_goal){	
	/* Function for calculating the new flywheel speed*/
 
	mtr_new_speed = (int)(mtr_speed + kd * (omega - omega_goal));	
	if (mtr_new_speed > mtr_max_speed)
	{
		mtr_new_speed = mtr_max_speed;
	}
	else if (mtr_new_speed < -mtr_max_speed)
	{
		mtr_new_speed = -mtr_max_speed;
	}
	return mtr_new_speed;
}
 
void initialize_all(void){
/*	 All systems’ initialization function*/
	printf ("Enable motor №%d\n", mtr_num);
	motor_turn_on(mtr_num);
	Sleep(1);
	printf("Enable angular velocity sensor №%d\n", hyr_num); 
	hyro_turn_on(hyr_num); // Включаем ДУС
	Sleep(1);
	printf("Enable magnetometer %d\n", mag_num);
	magnetometer_turn_on(mag_num);
	Sleep(1);
}
 
int switch_off_all(){
/*	 All systems’ switching off function*/
	printf("Finishing...");
	int16_t new_speed = mtr_new_speed; 
	printf("\nDisable angular velocity sensor №%d\n", hyr_num);
	hyro_turn_off(hyr_num);
	printf("Disable magnetometer %d\n", mag_num);
	magnetometer_turn_off(mag_num);
	motor_set_speed(mtr_num, 0, &new_speed);
	Sleep (1);
	motor_turn_off(mtr_num);
	printf("Finish program\n");
	return 0;
}
 
int control(){
	float mtr_new_speed;
	//magnetometer data
	int16_t mgx_cal=0;
	int16_t mgy_cal=0;
	int16_t mgz_cal=0;
	int16_t *magx_raw = &mgx_cal;
	int16_t *magy_raw = &mgy_cal;
	int16_t *magz_raw = &mgz_cal;
	//angular velocity sensor data
	int16_t gx_raw;
	int16_t gy_raw;
	int16_t gz_raw;
	int16_t *hyrox_raw=&gx_raw;
	int16_t *hyroy_raw= &gy_raw;
	int16_t *hyroz_raw = &gz_raw;
	int16_t mtr_speed;
	int16_t omega;
	int16_t omega_goal=0;  // Target angular speed
 
	initialize_all();
 
	int mtr_state = 0;		// Initialize flywheel status
	int hyro_state = 0; 		// Initialize angular velocity sensor status
	int mag_state = 0; 		// Initialize magnetometer status
	int alpha_goal = 0;		// Target angle	
	double mag_alpha = 0;
	int i;
	for (i = 0; i < 500; i++)	{
		mag_state = magnetometer_request_raw(mag_num, magx_raw, magy_raw, magz_raw);
		hyro_state = hyro_request_raw(hyr_num,hyrox_raw,hyroy_raw,hyroz_raw); 
		gx_raw = *hyrox_raw; 
		gy_raw = *hyroy_raw; 
		gz_raw = *hyroz_raw; 
		mtr_state = motor_request_speed(mtr_num, &mtr_speed);
		if (!mag_state){
			mag_calibrated(magx_raw,magy_raw,magz_raw);
			*magy_raw = - *magy_raw;/* transition from the left coordinate system which is plotted on the magnetometer, to the right coordinate system to position the positive angle direction counter clock-wise*/
			mag_alpha = atan2(mgy_cal, mgx_cal)/M_PI*180;
			printf("magx_cal = %d", mgx_cal);
			printf(" magy_cal = %d", mgy_cal);
			printf(" magz_cal = %d", mgz_cal);
			printf(" mag_alpha atan2 = %f\n", mag_alpha);
		}
		else if (mag_state == 1){
			printf("Fail because of access error, check the connection\n");
		   }
		else if (mag_state == 2){
			printf("Fail because of interface error, check your code\n");
		  }
 
	if (!hyro_state){
			float gx_degs = gx_raw * 0.00875;
			float gy_degs = gy_raw * 0.00875;
			float gz_degs = gz_raw * 0.00875;
			/* if the angular velocity sensor sensor arranged via z-axis up, then satellite’s angular velocity matches to sensors’ readings of z-axis */
			omega = gz_degs;
			printf("\ngx_degs = %f", gx_degs);
			printf(" gy_degs = %f", gy_degs);
			printf(" gz_degs = %f\n", gz_degs);
		}
		else if (hyro_state == 1){
			printf("Fail because of access error, check the connection\n");
		}
		else if (hyro_state == 2){
			printf("Fail because of interface error, check your code\n");
		}
 
		if (!mtr_state)	{
			// if the flywheel returned error code 0 (no error)
			int16_t mtr_speed=0;
			motor_request_speed(mtr_num, &mtr_speed);
			printf("\nMotor_speed: %d\n", mtr_speed);	
			// setting a new flywheel speed
			mtr_new_speed = motor_new_speed_PD(mtr_speed,omega,omega_goal);
			motor_set_speed(mtr_num, mtr_new_speed, &omega);
		}	
		Sleep(time_step);	
	}
	switch_off_all();
	return 0;
}