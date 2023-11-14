#include <stdio.h>
#include <stdint.h>
#include "libschsat.h"
#define LSS_OK 0 
#define LSS_ERROR 1 
#define LSS_BREAK 2
#include <math.h> 
#include <time.h>
 
//Differential feedback coefficient.
const float kd = 200.0;
 
// Algorithm working time step, s
const float time_step = 0.1;
 
// Satellite’s target angular velocity, deg/s. For stabilization mode it is 0.0
const float omega_goal = 0.0;
// Maximum allowed flywheel speed, rev/min
const int mtr_max_speed = 5000;
const uint16_t mtr_num = 1;			// Flywheel number
const uint16_t hyr_num = 1; 		        // Angular velocity sensors’ number
const uint16_t mag_num = 1;			// Magnetometer number
// Measuring result’s number
int i = 1;
// Current turning angle
const float alpha = 0.0;
 
void initialize_all(void){/* // The function will switch on all apparatuses that will be used in the main program.
	printf("Enable angular velocity sensor №%d\n", hyr_num); 
	hyro_turn_on(hyr_num);
	Sleep(1);
	printf("Enable magnetometer %d\n", mag_num);
	magnetometer_turn_on(mag_num);
	Sleep(1); // Wait 1 second for switching on 
	printf("Enable Sun sensors 1-4\n");
	sun_sensor_turn_on(1);
	sun_sensor_turn_on(2);
	sun_sensor_turn_on(3);
	sun_sensor_turn_on(4);
	Sleep(1);
	printf("Enable motor №%d\n", mtr_num); 
	motor_turn_on(mtr_num);
	Sleep(1);
}
 
void switch_off_all(void){ // The function will switch off all apparatuses, that will be used in the main program.
	printf("Finishing...");
	int16_t new_speed = 0;
	hyro_turn_off(hyr_num);
	magnetometer_turn_off(mag_num);
	sun_sensor_turn_off(1);
	sun_sensor_turn_off(2);
	sun_sensor_turn_off(3);
	sun_sensor_turn_off(4);
	motor_set_speed(mtr_num, 0, &new_speed);
	Sleep(1);
	motor_turn_off(mtr_num);
	printf("\nFinish program\n");
}
 
int mag_calibrated(int16_t *magx, int16_t *magy, int16_t *magz ){
	/* instead of these 3 lines of code with calibration factors 
there must be the lines with calibration factors of your magnetometer */
	//magx_cal = 1.04*magx - 0.26*magy + 0.05*magz - 68.76		// it is the 1st line that must be changed with results of your magnetometer calibration 
	//magy_cal = 0.24*magx + 1.04*magy + 0.29*magz + 256.92	// it is the 2nd line that must be changed with results of your magnetometer calibration а
	//magz_cal = -0.09*magx - 0.19*magy + 0.77*magz + 159.41	// it is the 3rd  line that must be changed with results of your magnetometer calibration
	float magx_cal;
	float magy_cal;
	float magz_cal;
	magx_cal = 1.06*(*magx + -7.49) + -0.01*(*magy + -23.59) + 0.07*(*magz + -108.24);
	magy_cal = -0.01*(*magx + -7.49) + 1.11*(*magy + -23.59) + 0.09*(*magz + -108.24);
	magz_cal = 0.07*(*magx + -7.49) + 0.09*(*magy + -23.59) + 1.00*(*magz + -108.24);
	*magx = magx_cal;
	*magy = magy_cal;
	*magz = magz_cal;
	return 0;
}
 
int motor_new_speed_PD(int mtr_speed, float omega, int16_t omega_goal){	
/* 
Functions that defines flywheel’s new speed.
Flywheel’s new speed will be the sum of 
flywheel’s current speed and speed increment.
Speed increment is proportioned by angle error
and angular velocity error.
mtr_speed - flywheel’s current angular speed, rev/min
omega – satellite’s current angular speed, deg/s
omega_goal - target satellite’s angular speed, deg/s
mtr_new_speed - target flywheel’s angular speed, rev/min
*/
	int16_t mtr_new_speed;
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
 
int control(){// Program’s main function that will call other functions.
	int16_t omega;
	int omega_goal = 0;		// omega_goal - target satellite’s angular speed, deg/s
	initialize_all();		
	int mtr_state = 0;		// Initialize flywheel’s status	
	int hyro_state = 0;		// Initialize angular velocity sensor’s status
	int mag_state = 0; 		// Initialize magnetometer
	int16_t mtr_speed;
	int16_t mtr_new_speed;
	//angular velocity sensor’s data
	int16_t gx_raw;
	int16_t gy_raw;
	int16_t gz_raw;
	int16_t *hyrox_raw=&gx_raw;
	int16_t *hyroy_raw= &gy_raw;
	int16_t *hyroz_raw = &gz_raw;
	//magnetometer data
	int16_t mgx_cal=0;
	int16_t mgy_cal=0;
	int16_t mgz_cal=0;
	int16_t *magx_raw = &mgx_cal;
	int16_t *magy_raw = &mgy_cal;
	int16_t *magz_raw = &mgz_cal;
 
	float gx_degs;
	float gy_degs;
	float gz_degs;
	uint16_t sun_result_1[] = {0,0,0}; // Initialize sun_result_1
	uint16_t sun_result_2[] = {0,0,0}; // Initialize sun_result_2
	uint16_t sun_result_3[] = {0,0,0}; // Initialize sun_result_3
	uint16_t sun_result_4[] = {0,0,0}; // Initialize sun_result_4
	int mag_alpha = 0;
	const int sizeOD = 10;
    int sizeODA = 0;
    int tempSODA;
//	double* output_data = (double*)calloc(sizeOD, sizeof(double));
    double* output_data_all = (double*)calloc(sizeODA, sizeof(double));
 
	// Measuring result’s number
	int i = 0;
	// Put into memory rotation start time 
	long int time_start = time(NULL);
	// Put into memory one-second interval start time 
	long int time_interval = time(NULL);
	// solar sensors data output interval in seconds 
	int time_output = 0.1;
	int j;
	char a=1;
 
	while (a==1){
		// angular velocity sensor and flywheel data request
		hyro_state = hyro_request_raw(hyr_num,hyrox_raw,hyroy_raw,hyroz_raw); 
		mtr_state = motor_request_speed(mtr_num, &mtr_speed);
		mag_state = magnetometer_request_raw(mag_num, magx_raw, magy_raw, magz_raw);
 
 
		if (!hyro_state){
			 /*Processing the readings of angular velocity sensor,
		 calculation of satellite angular velocity upon angular velocity sensor’s readings.
		 If angular velocity sensor’s error code is 0 (no error) */
			gx_degs = gx_raw * 0.00875; 
			gy_degs = gy_raw * 0.00875;
			gz_degs = gz_raw * 0.00875;
			/* if angular velocity sensor arranged via z-axis up, then satellite’s angular velocity
			 matches to sensors’ readings of z-axis, otherwise 
			 it is required to reverse the sign: omega = - gz_degs */
			omega = gz_degs;
//			printf("gx_degs=%f, gy_degs=%f, gz_degs=%f\n", gx_degs, gy_degs, gz_degs);//ну так на всякий 
		}
		else if (hyro_state == 1){
			printf("Fail because of access error, check the connection\n");
		}
		else if (hyro_state == 2) {
			printf("Fail because of interface error, check your code\n");
		}
 
 
		//Processing the readings of flywheel and setting the target angular velocity.	
		if (!mtr_state)	{// if error code 0 (no error)
			int16_t mtr_speed=0;
			motor_request_speed(mtr_num, &mtr_speed);
//			printf("Motor_speed: %d\n", mtr_speed);	
			// set flywheel’s new speed 
			mtr_new_speed = motor_new_speed_PD(mtr_speed,omega,omega_goal);
			motor_set_speed(mtr_num, mtr_new_speed, &omega);
		}
 
		Sleep(time_step);
		long int time_current = time(NULL) - time_start;
 
		if (!mag_state){
			mag_calibrated(magx_raw,magy_raw,magz_raw);
			*magy_raw = - *magy_raw; /*transition from the left coordinate system which is plotted on the magnetometer, to the right coordinate system to position the positive angle direction counter clock-wise*/			
			mag_alpha = atan2(mgy_cal, mgx_cal)/M_PI*180;
		}
 
		if ((time(NULL) - time_interval) > time_output){
			// Put into memory next one-second interval starting time 
			time_interval = time(NULL);
			sun_result_1[0] = sun_sensor_request_raw(1, &sun_result_1[1],&sun_result_1[2]);
			sun_result_2[0] = sun_sensor_request_raw(2,&sun_result_2[1],&sun_result_2[2]);
			sun_result_3[0] = sun_sensor_request_raw(3,&sun_result_3[1],&sun_result_3[2]);
			sun_result_4[0] = sun_sensor_request_raw(4,&sun_result_4[1],&sun_result_4[2]);
 
			int output_data[] = {time_current, sun_result_1[1], sun_result_1[2], sun_result_2[1], sun_result_2[2], sun_result_3[1], sun_result_3[2], sun_result_4[1], sun_result_4[2], mag_alpha};
			tempSODA = sizeODA;
			sizeODA += sizeOD;
			output_data_all = (double*)realloc(output_data_all, sizeODA*sizeof(double));
			for (j=tempSODA; j<sizeODA; j++) {
				output_data_all[j] = output_data[j-sizeODA+1];
			}
		}		
		if (i > 100){  // Start rotation upon 5 seconds delay from the moment of launching
			omega_goal = 6.0;  // omega_goal - omega_goal – satellite’s target angular velocity, deg/s
		}
 
		if 	(time_current > 90){
			break;	
		}
		i += 1;	
	} 
 
	switch_off_all();
 
	printf("time_end = %ld" , time(NULL) - time_start);
 
	for (i = 0; i < 5000; i = i + 10){
		printf("%f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n",output_data_all[i], output_data_all[i+1], output_data_all[i+2], output_data_all[i+3], output_data_all[i+4], output_data_all[i+5], output_data_all[i+6], output_data_all[i+7], output_data_all[i+8], output_data_all[i+9]);
	}
	printf ("Ok\n");
	return 0;
}