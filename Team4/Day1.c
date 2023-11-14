#include "libschsat.h"

// Function to unwind the SC body to the left (clockwise) with a constant angular velocity
void unwindLeft() {
    const int num = 1;  // Assuming magnetometer #1 is used
    float angularVelocity = 6.0;  // deg/sec

    // Enable magnetometer
    magnetometer_turn_on(num);

    // Rotate SC body to the left
    motor_set_speed(num, angularVelocity, NULL);

    // Keep rotating for a specified duration (e.g., 10 seconds)
    mSleep(10000);

    // Stop rotation
    motor_set_speed(num, 0, NULL);

    // Disable magnetometer
    magnetometer_turn_off(num);
}

// Function to unwind the SC body to the right (counterclockwise) with a constant angular velocity
void unwindRight() {
    const int num = 1;  // Assuming magnetometer #1 is used
    float angularVelocity = -6.0;  // deg/sec (negative for counterclockwise)

    // Enable magnetometer
    magnetometer_turn_on(num);

    // Rotate SC body to the right
    motor_set_speed(num, angularVelocity, NULL);

    // Keep rotating for a specified duration (e.g., 10 seconds)
    mSleep(10000);

    // Stop rotation
    motor_set_speed(num, 0, NULL);

    // Disable magnetometer
    magnetometer_turn_off(num);
}



// Function to stabilize the satellite with locked SBs and capture control images
void stabilizeAndCaptureImages() {
    const int stabilizationTime = 10;  // seconds
    const int numImages = 3;

    // Lock SBs for stabilization
    // ...

    // Wait for stabilization
    Sleep(stabilizationTime);

    // Obtain control images from the camera
    for (int i = 0; i < numImages; ++i) {
        camera_take_photo(1);  // Assuming camera #1 is used
    }

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

int main() {
    // Perform tasks
    const char *textMessage = "Hello, receiving station! This is a test message.";

    

    unwindLeft();
    unwindRight();
    // Perform VHF transmission and TCS operation check
    transmitTextAndCheckTCS(textMessage);
    stabilizeAndCaptureImages();

    return 0;
}
