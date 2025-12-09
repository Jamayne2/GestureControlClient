// imu_filter.h
#ifndef IMU_FILTER_H
#define IMU_FILTER_H


#include "mpu6050.h"

// Expose raw data and filtered angle
extern fix15 acceleration[3];
extern fix15 gyro[3];
extern fix15 complementary_angle;  // tilt angle in degrees (fix15)


// Initialize IMU and filter state
void imu_filter_init(void);

// Read IMU and update complementary_angle
void imu_update_and_filter(void);

#endif
