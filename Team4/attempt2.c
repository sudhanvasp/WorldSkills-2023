#include "libschsat.h"
#define mtr_max_speed 5000
#define mtr_num 1
#define gyr_num 1
// Function to set the flywheel speed based on the angular velocity error
int setFlywheelSpeed(int currentSpeed, float angularVelocity, float goalAngularVelocity) {
    const float kd = 200.0;

    int newSpeed = (int)(currentSpeed + kd * (angularVelocity - goalAngularVelocity));

    if (newSpeed > mtr_max_speed) {
        newSpeed = mtr_max_speed;
    } else if (newSpeed < -mtr_max_speed) {
        newSpeed = -mtr_max_speed;
    }

    return newSpeed;
}

// Function to perform satellite unwinding and stabilization
void performUnwindingAndStabilization(float goalAngularVelocity) {
    // Initialize all devices
    initialize_all();

    // Set the initial state for the control loop
    int16_t mtr_state;
    int16_t gyro_state = 0;
    int16_t pRAW_dataX, pRAW_dataY, pRAW_dataZ;
    int16_t *gx_raw = &pRAW_dataX;
    int16_t *gy_raw = &pRAW_dataY;
    int16_t *gz_raw = &pRAW_dataZ;
    int16_t speed = 0;
    int16_t *mtr_speed = &speed;
    int16_t omega;

    // Main control loop
    for (int i = 0; i < 100; ++i) { // Adjust loop count as needed
        // Poll angular velocity sensor and flywheel
        gyro_state = gyro_request_raw(gyr_num, gx_raw, gy_raw, gz_raw);
        mtr_state = motor_request_speed(mtr_num, mtr_speed);

        // Process sensor readings
        if (!gyro_state) {
            float gx_degs = *gx_raw * 0.00875;
            float gy_degs = *gy_raw * 0.00875;
            float gz_degs = *gz_raw * 0.00875;

            // Adjust omega calculation based on sensor orientation
            omega = gz_degs;

            printf("gx_degs = %f, gy_degs = %f, gz_degs = %f\n", gx_degs, gy_degs, gz_degs);
        } else if (gyro_state == 1) {
            printf("Fail because of access error, check the connection\n");
        } else if (gyro_state == 2) {
            printf("Fail because of interface error, check your code\n");
        }

        // Process flywheel readings and set the required angular velocity
        if (!mtr_state) {
            int16_t currentSpeed = 0;
            motor_request_speed(mtr_num, &currentSpeed);

            printf("Motor_speed: %d\n", currentSpeed);

            int newSpeed = setFlywheelSpeed(currentSpeed, omega, goalAngularVelocity);

            // Set the new flywheel speed
            motor_set_speed(mtr_num, newSpeed, &omega);
        }

        // Sleep for a short duration (adjust as needed)
        mSleep(100);
    }

    // Switch off all devices
    switch_off_all();
}

int main() {
    // Unwind to the left (clockwise)
    printf("Unwinding to the left...\n");
    performUnwindingAndStabilization(5.0);

    // Unwind to the right (counterclockwise)
    printf("Unwinding to the right...\n");
    performUnwindingAndStabilization(-5.0);

    // Stabilize and capture images
    printf("Stabilizing and capturing images...\n");
    performUnwindingAndStabilization(0.0);

    return 0;
}
