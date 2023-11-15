import math
from time import sleep

time_step = 0.1
omega_goal = 0.0
mtr_max_speed = 3500
mtr_num = 1
hyr_num = 1
mag_num = 1
alpha_goal = 0
#bala changed nothing here:)
errora = 0
Pa = 0
Da = 0
Ia = 0
Kpa = 20
Kda = 0.1
Kia = 1
lastErrora = 0
Integratora = 0
PID_alpha = 0
Integrator_maxa = 10
Integrator_mina = -10

error = 0
P = 0
D = 0
I = 0
Kp = 20
Kd = 0.1
Ki = 1
lastError = 0
Integrator = 0
PID_omega = 0
Integrator_max = 10
Integrator_min = -10


def mag_calibrated(magx, magy, magz):
    magx_cal = 1.668867 * (magx + -38.326818) + -0.007649 * (magy + 20.442007) + 0.113348 * (magz + -69.659602)
    magy_cal = -0.007649 * (magx + -38.326818) + 1.749597 * (magy + 20.442007) + 0.130999 * (magz + -69.659602)
    magz_cal = 0.113348 * (magx + -38.326818) + 0.130999 * (magy + 20.442007) + 1.577785 * (magz + -69.659602)

    return magx_cal, magy_cal, magz_cal


def motor_new_speed_PD(mtr_speed, alpha, alpha_goal, omega, omega_goal):
    global Integrator
    global lastError
    global Integratora
    global lastErrora

    error = omega - omega_goal
    P = Kp * error
    D = Kd * (error - lastError) / time_step
    lastError = error
    Integrator = Integrator + error * time_step
    if Integrator > Integrator_max:
        Integrator = Integrator_max
    elif Integrator < Integrator_min:
        Integrator = Integrator_min
    I = Integrator * Ki
    PID_omega = P + I + D

    errora = alpha - alpha_goal
    Pa = Kpa * errora
    Da = Kda * (errora - lastErrora) / time_step
    lastErrora = errora
    Integratora = Integratora + errora * time_step
    if Integratora > Integrator_maxa:
        Integratora = Integrator_maxa
    elif Integratora < Integrator_mina:
        Integratora = Integrator_mina
    Ia = Integratora * Kia
    PID_alpha = Pa + Ia + Da

    mtr_new_speed = int(mtr_speed + PID_omega + PID_alpha)
    if mtr_new_speed > mtr_max_speed:
        mtr_new_speed = mtr_max_speed
    elif mtr_new_speed < -mtr_max_speed:
        mtr_new_speed = -mtr_max_speed
    print("P=", P, "\t\tI=", I, "\t\tD=", D, "\t\tPID_omega=", PID_omega, "\tmtr_new_speed=", mtr_new_speed)
    print("Pa=", Pa, "\tIa=", Ia, "\t\tDa=", Da, "\tPID_alpha=", PID_alpha)
    return mtr_new_speed


def initialize_all():
    print("Enable motor №", mtr_num)
    # motor_turn_on(mtr_num)  # You need to implement motor_turn_on function
    sleep(1)
    print("Enable angular velocity sensor №", hyr_num)
    # hyro_turn_on(hyr_num)  # You need to implement hyro_turn_on function
    sleep(1)
    print("Enable magnetometer", mag_num)
    # magnetometer_turn_on(mag_num)  # You need to implement magnetometer_turn_on function
    sleep(1)
    # camera_turn_on()  # You need to implement camera_turn_on function
    # transmitter_turn_on(1)  # You need to implement transmitter_turn_on function
    sleep(1)


def switch_off_all():
    print("Finishing...")
    # hyro_turn_off(hyr_num)  # You need to implement hyro_turn_off function
    # magnetometer_turn_off(mag_num)  # You need to implement magnetometer_turn_off function
    # motor_set_speed(mtr_num, 0)  # You need to implement motor_set_speed function
    sleep(1)
    # motor_turn_off(mtr_num)  # You need to implement motor_turn_off function
    # camera_turn_off()  # You need to implement camera_turn_off function
    # transmitter_turn_off(1)  # You need to implement transmitter_turn_off function
    sleep(1)
    print("Finish program")


def angle_transformation(alpha, alpha_goal):
    if alpha <= (alpha_goal - 180):
        alpha = alpha + 360
    elif alpha > (alpha_goal + 180):
        alpha = alpha - 360
    return alpha


def control():
    initialize_all()
    mtr_state = 0
    hyro_state = 0
    mag_state = 0
    alpha_goal = -120
    omega_goal = 0
    mag_alpha = 0
    frame = 0
    err = 0
    omega = 0

    sleep(22)

    for i in list(range(60)):
        mag_state, magx_raw, magy_raw, magz_raw = (0, 0, 0, 0)  # Replace with actual magnetometer readings
        hyro_state, gx_raw, gy_raw, gz_raw = (0, 0, 0, 0)  # Replace with actual hyro readings
        mtr_state, mtr_speed = (0, 0)  # Replace with actual motor readings

        if not mag_state:
            magx_cal, magy_cal, magz_cal = mag_calibrated(magx_raw, magy_raw, magz_raw)
            mag_alpha = math.atan2(magy_cal, magx_cal) / math.pi * 180
            mag_alpha = angle_transformation(mag_alpha, alpha_goal)
            print("mag_alpha atan2=", mag_alpha)
        elif mag_state == 1:
            print("Fail because of access error, check the connection")
        elif mag_state == 2:
            print("Fail because of interface error, check your code")

        if not hyro_state:
            gz_degs = gz_raw * 0.00875
            omega = gz_degs
        elif hyro_state == 1:
            print("Fail because of access error, check the connection")
        elif hyro_state == 2:
            print("Fail because of interface error, check your code")

        if not mtr_state:
            print("Motor_speed:", mtr_speed, "\t\ti=", i, end=' ')
            mtr_new_speed = motor_new_speed_PD(mtr_speed, mag_alpha, alpha_goal, omega, omega_goal)
            # motor_set_speed(mtr_num, mtr_new_speed)  # Uncomment this line once motor_set_speed is implemented

        if i == 50:
            # err_camera = camera_take_photo(frame)  # Uncomment this line once camera_take_photo is implemented

            if err == 1:
                print('Camera access error, check the connection', frame)

            elif err == 2:
                print('Camera interface error, check your code', frame)

            # err_transmitter = transmitter_transmit_photo(1, frame)  # Uncomment this line once transmitter_transmit_photo is implemented

            if err_transmitter == 1:
                print('Transmission failed')

        sleep(time_step)

    switch_off_all()


# Uncomment the following lines once the respective functions are implemented
# motor_turn_on, hyro_turn_on, magnetometer_turn_on, camera_turn_on,
# transmitter_turn_on, hyro_turn_off, magnetometer_turn_off,
# motor_set_speed, motor_turn_off, camera_turn_off, transmitter_turn_off,
# camera_take_photo, transmitter_transmit_photo

# control()  # Uncomment this line to execute the control function

