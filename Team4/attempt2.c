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



void transmitTextAndCheckTCS(const char *text) {
    const uint16_t txNum = 1;  // Assuming transceiver #1 is used
    const uint16_t rxNum = 2;  // Assuming receiving station's transceiver #2
    const uint16_t maxMessageSize = 256;  // Adjust the size based based on trials

    // Check if the message size is within the allowed limit
    if (strlen(text) > maxMessageSize) {
        printf("Error: Message size exceeds the allowed limit.\n");
        return;
    }

    // Convert the text message to uint8_t array (assuming ASCII characters)
    const uint8_t *messageData = (const uint8_t *)text;

    // Transmit the text message via VHF channel
    int sendResult = transceiver_send(txNum, rxNum, messageData, strlen(text));

    // Check the result of the transmission
    if (sendResult == LSS_OK) {
        printf("Text transmitted successfully via VHF channel.\n");
    } else {
        printf("Error: Failed to transmit text via VHF channel.\n");
        return;
    }


    printf("TCS operation check completed.\n");
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
    // Perform tasks
    const char *textMessage = "Hello, receiving station! This is a test message.";

    // Unwind to the left (clockwise)
    printf("Unwinding to the left...\n");
    performUnwindingAndStabilization(7.0);


    // Unwind to the right (counterclockwise)
    printf("Unwinding to the right...\n");
    performUnwindingAndStabilization(-7.0);


    // Perform VHF transmission and TCS operation check
    transmitTextAndCheckTCS(textMessage);


    // Stabilize and begin capturing images
    printf("Stabilizing and capturing images...\n");
    performUnwindingAndStabilization(0.0);
    const int stabilizationTime = 10;  // seconds
    const int numImages = 5;

    Sleep(stabilizationTime);

    // Obtain control images from the camera
    for (int i = 0; i < numImages; ++i) {
        camera_take_photo(1);  // Assuming camera #1 is used
        transmitter_transmit_photo(1);  //transmit it to the ground station
        Sleep(500); //adjust as needed
    }


    return 0;
}
