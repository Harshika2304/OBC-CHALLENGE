 
#ifndef SPACE_ADCS_APP_H
#define SPACE_ADCS_APP_H

#include "cfe.h"
#include <stdint.h>  // Ensure fixed-width integer types are defined

#ifdef __cplusplus
extern "C" {
#endif

// Sensor data structure
typedef struct {
    float gyro[3];            // Gyroscope readings (rad/s)
    float magnetometer[3];    // Magnetometer readings (Gauss or Tesla)
    float sun_angle;          // Sun sensor angle (degrees or radians)
    uint32_t timestamp;       // Timestamp in microseconds or milliseconds
    uint8_t valid;            // Validity flag: 0 = invalid, 1 = valid
} SensorData_t;

// Actuator commands structure
typedef struct {
    float wheel_torques[3];   // Reaction wheel torques (Nm)
    float magnetorquer[3];    // Magnetorquer currents (A)
    uint32_t timestamp;       // Timestamp in microseconds or milliseconds
} ActuatorCommands_t;

// Main entry point for Space ADCS application
void SpaceADCS_AppMain(void);

#ifdef __cplusplus
}
#endif

#endif
