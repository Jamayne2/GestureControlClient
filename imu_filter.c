// imu_filter.c
#include <math.h>
#include "imu_filter.h"
#include "mpu6050.h"

// Raw IMU measurements (15.16 fixed-point, accel in g, gyro in deg/s)
fix15 acceleration[3], gyro[3];

// Filter state
fix15 accel_angle, gyro_angle_delta;
fix15 complementary_angle;
fix15 filter_acc_z, filter_acc_y;

// These are defined in some common file in your project, e.g. globals.c
// fix15 oneeightyoverpi = float2fix15(180.0f / 3.14159265358979f);
// fix15 zeropt001       = float2fix15(0.001f);
// fix15 zeropt999       = float2fix15(0.999f);

void imu_filter_init(void)
{
    // Make sure MPU is already reset/configured before calling this
    mpu6050_read_raw(acceleration, gyro);

    filter_acc_y        = acceleration[1];
    filter_acc_z        = acceleration[2];
    accel_angle         = 0;
    gyro_angle_delta    = 0;
    complementary_angle = 0;
}

static void complementary_filter(void)
{
    // Low-pass filter Y and Z acceleration (simple IIR with >>4 ~ 1/16)
    filter_acc_y = filter_acc_y + ((acceleration[1] - filter_acc_y) >> 4);
    filter_acc_z = filter_acc_z + ((acceleration[2] - filter_acc_z) >> 4);

    // Accelerometer angle (atan2 in radians -> degrees)
    accel_angle = multfix15(
        float2fix15(atan2(-filter_acc_y, filter_acc_z)),
        oneeightyoverpi
    );

    // Gyro angle increment (gyro[0] in deg/s * dt ≈ 0.001 s)
    gyro_angle_delta = multfix15(gyro[0], zeropt001);

    // Complementary filter
    complementary_angle =
        multfix15(complementary_angle - gyro_angle_delta, zeropt999) +
        multfix15(accel_angle, zeropt001);
}

void imu_update_and_filter(void)
{
    // 1) Read raw IMU
    mpu6050_read_raw(acceleration, gyro);

    // 2) Update complementary_angle
    complementary_filter();
}
