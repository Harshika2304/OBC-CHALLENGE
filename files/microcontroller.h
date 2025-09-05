#ifndef MICROCONTROLLER_H
#define MICROCONTROLLER_H

#include "adcs_controller.h"
#include <cstdint>  // For uint32_t, uint8_t
#include <cstddef>  // for nullptr_t if needed in implementations

struct SensorData {
    float gyro[3];
    float magnetometer[3];
    float sun_angle;
    uint32_t timestamp;
    bool valid;
};

struct ActuatorCommands {
    float wheel_torques[3];
    float magnetorquer[3];
    uint32_t timestamp;
};

class Microcontroller {
public:
    Microcontroller();
    ~Microcontroller();

    // Process incoming sensor sample and update controller outputs
    void processSensorData(const SensorData& sensor_data);

    // Copy current actuator commands into the provided struct
    void getActuatorCommands(ActuatorCommands& commands) const;

    // Control / configuration
    void setControlMode(int mode);
    void setFaultThreshold(float threshold);

    // Health / telemetry getters
    uint8_t getFaultFlags() const { return fault_flags_; }
    uint32_t getControlCycles() const { return control_cycles_; }
    int getControlMode() const { return control_mode_; }

private:
    void performFaultDetection();
    void updateHealthMonitoring();

    ADCSController* adcs_controller_;

    // Sensor data (latest)
    float gyro_rates_[3];
    float magnetometer_[3];
    float sun_angle_;

    // Actuator outputs (latest)
    float wheel_torques_[3];
    float magnetorquer_[3];

    // Control & health state
    int control_mode_;
    uint8_t fault_flags_;
    uint32_t control_cycles_;
    uint32_t uptime_seconds_;

    // Tunable parameters
    float fault_threshold_;
    float max_torque_;

    // Fault counters and tracking
    uint32_t fault_count_;
    uint32_t consecutive_faults_;
};

#endif // MICROCONTROLLER_H
