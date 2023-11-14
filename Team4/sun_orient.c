#include "libschsat.h" 
#include <stdio.h> 
#include <stdint.h> 
#include "libschsat.h" 
#define LSS_OK 0 
#define LSS_ERROR 1 
#define LSS_BREAK 2 
#include <math.h>
 
const  int kd =  200 ;  /*Differential feedback kit.
If the angular velocity of the satellite is positive,
then the satellite must be rotated clockwise,
those. the flywheel must be accelerated clockwise.*/ 
const  int kp =  - 100 ;  /* Proportional feedback set.
If the current angle is greater than the target angle,
then the satellite must be rotated counterclockwise,
Accordingly, the flywheel must be accelerated counterclockwise.*/ 
const  float time_step =  0.05 ;  //Time step of the orientation and stabilization system 
const  int mtr_max_speed =  3000 ;  		// Maximum permissible flywheel speed, rpm 
const  uint16_t mtr_num =  1 ; 			// Flywheel number 
const  uint16_t hyr_num =  1 ;  		        // DUS number 
const  uint16_t mag_num =  1 ; 			// Magnetometer number
 
const  int sun_max =  25000 ; //The maximum value of the solar sensor, more than which we consider an error 
const  int sun_min =  50 ; //The minimum value of the solar sensor, less than which we consider an error 
int16_t mtr_new_speed ;
 
int angle_transformation ( int alpha ,  int alpha_goal ) { 
	/* To determine the control command for the flywheel, it is important to know not the angle of rotation of the satellite relative to the magnetic field, but the angular error - the difference between the current angle and the target angle alpha_goal
// By default, the angle is defined in the range from -180 to 180 degrees
// The angle_transformation function changes the angle measurement range. For solutions
orientation problems, the angle must be measured in the range from (alpha_goal-180) to
(alpha_goal+180), where alpha_goal is the target angle between the x-axis of the magnetometer
and the projection of the magnetic field vector onto the horizontal plane.*/ 
	if  ( alpha <=  ( alpha_goal -  180 ) ) { 
		alpha = alpha +  360 ; 
	} 
	else  if  ( alpha > ( alpha_goal + 180 ) ) { 
		alpha = alpha -  360 ; 
	} 
	return alpha ; 
}
 
int motor_new_speed_PD ( int mtr_speed , int alpha , int alpha_goal ,  float omega ,  int omega_goal ) { 
	/* Determine the new flywheel speed
// New flywheel speed equals current flywheel speed + speed increment
// The speed increment is proportional to the angle error and the angular velocity error*/ 
	int mtr_new_speed = mtr_speed + kp * ( alpha - alpha_goal )  + kd * ( omega - omega_goal ) ; 
	if  ( mtr_new_speed > mtr_max_speed ) { 
		mtr_new_speed = mtr_max_speed ; 
	} 
	else  if  ( mtr_new_speed <  - mtr_max_speed ) { 
		mtr_new_speed =  - mtr_max_speed ; 
	} 
	return mtr_new_speed ; 
}
 
void initialize_all ( void ) { /* The function includes all devices,
which will be used in the main program.*/ 
	printf ( "Enable motor №%d \n " , mtr_num ) ;  
	motor_turn_on ( mtr_num ) ; 
	sleep ( 1 ) ; 
	printf ( "Enable angular velocity sensor №%d \n " , hyr_num ) ;  
	hyro_turn_on ( hyr_num ) ;  // Turn on DUS 
	Sleep ( 1 ) ; 
	printf ( "Enable Sun sensors 1-4 \n " ) ; 
	sun_sensor_turn_on ( 1 ) ; 
	sun_sensor_turn_on ( 2 ) ; 
	sun_sensor_turn_on ( 3 ) ; 
	sun_sensor_turn_on ( 4 ) ; 
	sleep ( 1 ) ; 
}
 
void switch_off_all ( void ) { /* The function turns off all devices
 which will be used in the main program.*/ 
	printf ( "Finishing... \n " ) ; 
	int16_t new_speed = mtr_new_speed ; 
	hyro_turn_off ( hyr_num ) ;  // Turn off the DUS 
	magnetometer_turn_off ( mag_num ) ; 
	sun_sensor_turn_off ( 1 ) ; 
	sun_sensor_turn_off ( 2 ) ; 
	sun_sensor_turn_off ( 3 ) ; 
	sun_sensor_turn_off ( 4 ) ; 
	motor_set_speed ( mtr_num ,  0 ,  & new_speed ) ; 
	sleep ( 1 ) ; 
	motor_turn_off ( mtr_num ) ; 
	printf ( "Finish program \n " ) ; 
}
 
int check_sun ( int sun11 ,  int sun12 ,  int sun21 ,  int sun22 ,  int sun31 , int sun32 , int sun41 , int sun42 ) { int delta1 = 0 ; int delta2 = 0 ; int delta3 = 0 ; int delta4 = 0 ; int delta5 = 0 ; int delta6 = 0 ; int delta7 = 0 ; int delta8 = 0 ; int delta_sum = 36000 ; int delta_min = 35000 ; int delta_i = 0 ; float data_sun [ ] = { - 47.329501295977 , 239 , 102 , - 48.1458278265892 } ; //insert data from a text file with data int i ; for ( i = 0 ; i < 4 ; i ++ ) {  				 
		delta1 = abs ( sun11 - data_sun [ i ] ) ; 
		delta2 = abs ( sun12 - data_sun [ i ] ) ; 
		delta3 = abs ( sun21 - data_sun [ i ] ) ; 
		delta4 = abs ( sun22 - data_sun [ i ] ) ; 
		delta5 = abs ( sun31 - data_sun [ i ] ) ; 
		delta6 = abs ( sun32 -data_sun [ i ] ) ; 
		delta7 =  abs ( sun41 - data_sun [ i ] ) ; 
		delta8 =  abs ( sun42 - data_sun [ i ] ) ; 
		delta_sum = delta1 + delta2 + delta3 + delta4 + delta5 + delta6 + delta7 + delta8 ; 
		printf ( "%d, %f \n " , i , data_sun [ i ] ) ; 
		printf ( "Sun_sens: % d, %d, %d, %d, %d, %d, %d, %d \n " , sun11 , sun12 , sun21 , sun22 , sun31 , sun32 , sun41 , sun42 ) ; 
		printf ( "deltas= %d, %d, %d, %d, %d, %d, %d, %d, %d \n " , delta1 , delta2 , delta3 , delta4 , delta5 , delta6 , delta7 , delta8 , delta_sum ) ; 		
		if  ( delta_min > delta_sum )  { 
			delta_min = delta_sum ; 
			delta_i = data_sun [ i ] ; 
			printf ( "delta_min= %d \n " , delta_min ) ; 
			printf ( "delta_i= %d \n " , delta_i ) ; 
		} 
	} 
	return delta_i ; 
}
 
int sun_range ( uint16_t  * sun_1 ,  uint16_t  * sun_2 ,  uint16_t sun_old_1 ,  uint16_t sun_old_2 ) { 	
 
	if  ( * sun_1 > sun_max ) { 
		* sun_1 = sun_old_1 ; 
	}
 
	if  ( * sun_1 < sun_min ) { 
		* sun_1 = sun_old_1 ; 
	} 
	if  ( * sun_2 > sun_max ) { 
		* sun_2 = sun_old_2 ; 
	} 
	if  ( * sun_2 < sun_min ) { 
		* sun_2 = sun_old_2 ; 
	} 		
	return  0 ; 
}
 
int control ( ) {  // main function of the program 
	initialize_all ( ) ;  // Initialize the flywheel status 
	int mtr_state =  0 ;   // Initialize the status of the DUS	 
	int hyro_state =  0 ; 
	//int mag_state = 0; //Initialize the status of the magnetometer 
	uint16_t sun_result_1 [ ]  =  { 0 , 0 , 0 } ;  // Initialize sun_result_1 
	uint16_t sun_result_2 [ ]  =  { 0 , 0 , 0 } ;  // Initialize sun_result_2 
	uint16_t sun_result_3 [ ]  =  { 0 , 0 , 0 } ;  // Initialize sun_result_3 
	uint16_t sun_result_4 [ ]  =  { 0 , 0 , 0 } ;  // Initialize sun_result_4 
	uint16_t sun_result_1_Old [ ]  =  { 0 , 0 , 0 } ;  // Initialize sun_result_1 
	uint16_t sun_result_2_Old [ ]  =  { 0 , 0 , 0 } ;  // Initialize sun_result_2 
	uint16_t sun_result_3_Old [ ]  =  { 0 , 0 , 0 } ;  // Initialize sun_result_3 
	uint16_t sun_result_4_Old [ ]  =  { 0 , 0 , 0 } ;  // Initialize sun_result_4 
	int16_t mtr_speed ; 
	//DUS data 
	int16_t gx_raw ; 
	int16_t gy_raw ; 
	int16_t gz_raw ; 
	int16_t  * hyrox_raw =  & gx_raw ; 
	int16_t  * hyroy_raw =  & gy_raw ; 
	int16_t  *hyroz_raw =  & gz_raw ; 
	//magnetometer data 
	/*int16_t mgx;
	int16_t mgy;
	int16_t mgz;
	int16_t *magx_raw = &mgx;
	int16_t *magy_raw = &mgy;
	int16_t *magz_raw = &mgz;*/
 
	int16_t omega ; 
	int omega_goal =  0 ; 		// omega_goal - target angular velocity of the satellite, deg/s 
	int alpha_goal =  30 ; 		// Target rotation angle 
	int i ; 
	int j ; 
	for  ( i =  0 ; i <  300 ; i ++ ) { 
		// Remember the old values 
		​​for ( j =  0 ; j <  3 ; j ++ ) { 
		sun_result_1_Old [ j ]  = sun_result_1 [ j ] ; 	
		sun_result_2_Old [ j ]  = sun_result_2 [ j ] ; 		
		sun_result_3_Old [ j ]  = sun_result_3 [ j ] ; 		
		sun_result_4_Old [ j ]  = sun_result_4 [ j ] ; 
		} 		
		// polling of sensors and flywheel 
		sun_result_1 [ 0 ]  = sun_sensor_request_raw ( 1 ,& sun_result_1 [ 1 ] ,& sun_result_1 [ 2 ] ) ; 
		sun_result_2 [ 0 ]  = sun_sensor_request_raw ( 2 ,& sun_result_2 [ 1 ] ,& sun_result_2 [ 2 ] ) ; 			
		sun_result_3 [ 0 ]  = sun_sensor_request_raw ( 3 ,& sun_result_3 [ 1 ] ,& sun_result_3 [ 2 ] ) ; 
		sun_result_4 [ 0 ]  = sun_sensor_request_raw ( 4 ,& sun_result_4 [ 1 ] ,& sun_result_4 [ 2 ] ) ;
		//print sun_result_1, sun_result_2, sun_result_3, sun_result_4 
		// Check that the new measured value does not fall outside the range 
		sun_result_1 [ 0 ]  = sun_range (  & sun_result_1 [ 1 ] ,  & sun_result_1 [ 2 ] , sun_result_1_Old [ 1 ] , sun_result_1_Old [ 2 ] ) ; 
		sun_result_2 [ 0 ]  = sun_range (  & sun_result_2 [ 1 ] ,  & sun_result_2 [ 2 ] , sun_result_2_Old [ 1 ] , sun_result_2_Old [ 2 ] ) ; 
		sun_result_3 [ 0 ]  = sun_range (  & sun_result_3 [ 1 ] ,  & sun_result_3 [ 2 ] , sun_result_3_Old [ 1 ] , sun_result_3_Old [ 2 ] ) ; 
		sun_result_4 [ 0 ]  = sun_range (  & sun_result_4 [ 1 ] ,  & sun_result_4 [ 2 ] , sun_result_4_Old [ 1 ] , sun_result_4_Old [ 2 ] ) ; 
		int alpha_sun = check_sun ( sun_result_1 [ 1 ] , sun_result_1 [ 2 ] , sun_result_2 [ 1 ] , sun_result_2 [ 2 ] , sun_result_3 [ 1 ] , sun_result_3 [ 2 ] , sun_result_4 [ 1 ] , sun_result_4 [ 2 ] ) ; 
		alpha_sun = angle_transformation( alpha_sun , alpha_goal ) ; 
		printf ( "alpha_sun = %d \n " , alpha_sun ) ;
 
		hyro_state = hyro_request_raw ( hyr_num , hyrox_raw , hyroy_raw , hyroz_raw ) ;  
		gx_raw =  * hyrox_raw ;  
		gy_raw =  * hyroy_raw ;  
		gz_raw =  * hyroz_raw ;  
		mtr_state = motor_request_speed ( mtr_num ,  & mtr_speed ) ; 
		//mag_state = magnetometer_request_raw(mag_num, magx_raw, magy_raw, magz_raw); 
		mtr_speed = motor_request_speed ( mtr_num ,  & mtr_speed ) ;
 
 
		if  ( ! hyro_state ) {  // if the DUS returned error code 0, i.e. no error 
			float gx_degs = gx_raw *  0.00875 ; 
			float gy_degs = gy_raw *  0.00875 ; 
			float gz_degs = gz_raw *  0.00875 ; 
			omega =  ( int16_t ) gz_degs ; 	// if the DS is installed with the z axis up, then the angular velocity of the satellite coincides with the DS readings along the z axis 
			printf ( "gx_degs = %f, gy_degs = %f, gz_degs = %f \n " , gx_degs , gy_degs , gz_degs ) ;  // Output data 
		} 
		else  if  ( hyro_state ==  1 ) {  // if the sensor returned error message 1 
			printf ( "Fail because of access error, check the connection" ) ; 
		} 
		else  if  ( hyro_state ==  2 ) {  // if the sensor returned error message 2 
			printf ( "Fail because of interface error, check your code" ) ; 
		}
 
		if  ( ! mtr_state ) 	{  // if the error code is 0, i.e. no error 
			int16_t mtr_speed =  0 ; 
			motor_request_speed ( mtr_num ,  & mtr_speed ) ; 
			printf ( "Motor_speed: %d \n " , mtr_speed ) ; 	
			// setting new flywheel speed 
			mtr_new_speed = motor_new_speed_PD ( mtr_speed , alpha_sun , alpha_goal , omega , omega_goal ) ; 
			motor_set_speed ( mtr_num , mtr_new_speed ,  & omega ) ; 
		}	
 
		Sleep ( time_step ) ; 
	} 
	switch_off_all ( ) ; 
	return  0 ; 
}