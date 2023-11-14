#include "libschsat.h" 
const  float kd =  200.0 ;
 
// Time step of the algorithm, with 
const  float time_step =  0.2 ;
 
// Target angular velocity of the satellite, deg/s. 
// For stabilization mode it is equal to 0.0. 
const  float omega_goal =  0.0 ;
 
// Flywheel number 
const  int mtr_num =  1 ;
 
// Maximum permissible flywheel speed, rpm 
const  int mtr_max_speed =  5000 ;
 
// Number of the RUS (angular velocity sensor) 
const   uint16_t hyr_num =  1 ; 
int16_t mtr_new_speed ; 
int motor_new_speed_PD ( int mtr_speed ,  int omega ,  float omega_goal ) { 
 /* Function for determining the new flywheel speed.
 The new flywheel speed is made up of
 current flywheel speed and speed increment.
 The speed increment is proportional to the angle error and the angular velocity error.
 mtr_speed - current angular speed of the flywheel, rpm
 omega - current angular velocity of the satellite, deg/s
 omega_goal - target angular velocity of the satellite, deg/s
 mtr_new_speed - required flywheel angular speed, rpm*/
 
	mtr_new_speed =  ( int ) ( mtr_speed + kd *  ( omega - omega_goal ) ) ;
 
	if  ( mtr_new_speed > mtr_max_speed ) 
	{ 
		mtr_new_speed = mtr_max_speed ; 
	} 
	else  if  ( mtr_new_speed <  - mtr_max_speed ) 
	{ 
		mtr_new_speed =  - mtr_max_speed ; 
	} 
	return mtr_new_speed ; 
}
 
void initialize_all ( void ) { 
/*The function includes all devices that will be used in the main program.*/ 
	printf ( "Enable motor №%d \n " , mtr_num ) ;  
	motor_turn_on ( mtr_num ) ; 
	sleep ( 1 ) ; 
	printf ( "Enable angular velocity sensor №%d \n " , hyr_num ) ;  
	hyro_turn_on ( hyr_num ) ; 
	sleep ( 1 ) ; 
 }
 
void switch_off_all ( void ) { 
/* The function turns off all devices that will be used in the main program. */ 
	printf ( "Finishing..." ) ; 
	int16_t new_speed = mtr_new_speed ;  // 
	printf ( " \n Disable angular velocity sensor №%d \n " , hyr_num ) ; 
	hyro_turn_off ( hyr_num ) ; 
	motor_set_speed ( mtr_num ,  0 ,  & new_speed ) ; 
	sleep ( 1 ) ; 
	motor_turn_off ( mtr_num ) ; 
	printf ( "Finish program \n " ) ; 
}
 
void control ( void ) { 
	initialize_all ( ) ; 
	// Initialize the flywheel status 
	int16_t mtr_state ; 
	// Initialize the status of the DUS	 
	int16_t hyro_state =  0 ; 
	int16_t pRAW_dataX =  0 ; 
	int16_t pRAW_dataY =  0 ; 
	int16_t pRAW_dataZ =  0 ; 
	int16_t  * gx_raw =  & pRAW_dataX ;  
	int16_t  * gy_raw =  & pRAW_dataY ;  
	int16_t  * gz_raw =  & pRAW_dataZ ; 
	int16_t speed =  0 ; 
	int16_t 	* mtr_speed =  & speed ; 
	int16_t omega ; 
	int i ; 
	for ( i =  0 ; i <  1000 ; i ++ ) { 
		printf ( "i = %d \n " , i ) ; 
		// Poll of angular velocity sensor and flywheel. 
		hyro_state = hyro_request_raw ( hyr_num , gx_raw , gy_raw , gz_raw ) ;  	
		mtr_state = motor_request_speed ( mtr_num , mtr_speed ) ;
 
		// Processing the readings of the angular velocity sensor, 
		// calculating the angular velocity of the satellite based on the readings of the DUS. 
		// If the DUS error code is 0, i.e. no error 
		if  ( ! hyro_state ) { 
			float gx_degs =  * gx_raw *  0.00875 ; 
			float gy_degs =  * gy_raw *  0.00875 ; 
			float gz_degs =  * gz_raw *  0.00875 ; 
			// if the DS is installed with the z axis up, then the angular velocity 
			// of the satellite coincides with the DS readings along the z axis, otherwise 
			// it is necessary to change the sign: omega = - gz_degs 
			omega = gz_degs ; 
			printf ( "gx_degs = %f " , gx_degs ) ; 
			printf ( "gy_degs = %f " , gy_degs ) ; 
			printf ( " gz_degs = %f \n " , gz_degs ) ; 
		} 
		else  if  ( hyro_state ==  1 ) { 
			printf ( "Fail because of access error, check the connection \n " ) ; 
		} 
		else  if  ( hyro_state ==  2 ) { 
			printf ( "Fail because of interface error, check your code \n " ) ; 
		}
 
		int mtr_new_speed ; 
		//Processing flywheel readings and setting the required angular velocity. 
		if  ( ! mtr_state ) 	{ 
			// if the error code is 0, i.e. no error 
			int16_t mtr_speed = 0 ; 
			motor_request_speed ( mtr_num ,  & mtr_speed ) ; 
			printf ( "Motor_speed: %d \n " , mtr_speed ) ; 	
		// setting the new flywheel speed 
			mtr_new_speed = motor_new_speed_PD ( mtr_speed , omega , omega_goal ) ; 
			motor_set_speed ( mtr_num , mtr_new_speed ,  & omega ) ; 
		} 
	} 
	Sleep ( time_step ) ; 
	switch_off_all ( ) ; 
}

void main ( void ) { 
    control ( ) ; 
}