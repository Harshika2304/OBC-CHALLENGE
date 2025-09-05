#include "microcontroller.h"
#include "adcs_controller.h"
#include <iostream>
#include <cmath>
#include <cstring>
#include <algorithm>

Microcontroller::Microcontroller()
    : control_mode_(0),
      fault_flags_(0),
      control_cycles_(0),
      uptime_seconds_(0),
      fault_threshold_(0.5f),
      max_torque_(0.1f),
      fault_count_(0),
      consecutive_faults_(0),
      adcs_controller_(nullptr) // ensure deterministic initialization
{
    // allocate ADCS controller (original logic preserved)
    adcs_controller_ = new ADCSController(0.1f, 0.05f, 0.01f);

    // initialize sensor & actuator arrays to zero
    std::memset(gyro_rates_, 0, sizeof(gyro_rates_));
    std::memset(magnetometer_, 0, sizeof(magnetometer_));
    std::memset(wheel_torques_, 0, sizeof(wheel_torques_));
    std::memset(magnetorquer_, 0, sizeof(magnetorquer_));

    sun_angle_ = 0.0f;

    std::cout << "Microcontroller initialized - Safe Mode" << std::endl;
}

Microcontroller::~Microcontroller() {
    delete adcs_controller_;
    adcs_controller_ = nullptr;
}

void Microcontroller::processSensorData(const SensorData& sensor_data) {
    if (!sensor_data.valid) {
        consecutive_faults_++;
        if (consecutive_faults_ > 5) {
            control_mode_ = 0; // enter safe mode after repeated invalid sensor reads
        }
        return;
    }

    // copy sensor values in
    for (int i = 0; i < 3; ++i) {
        gyro_rates_[i] = sensor_data.gyro[i];
        magnetometer_[i] = sensor_data.magnetometer[i];
    }
    sun_angle_ = sensor_data.sun_angle;

    // reset consecutive fault counter on a valid read
    consecutive_faults_ = 0;

    // run fault detection (may modify control_mode_)
    performFaultDetection();

    // Only call computeControl if ADCS controller exists
    if (adcs_controller_) {
        adcs_controller_->computeControl(gyro_rates_, magnetometer_, sun_angle_,
                                         wheel_torques_, magnetorquer_, control_mode_);
    } else {
        // Defensive: zero-out outputs if ADCS controller isn't present
        std::memset(wheel_torques_, 0, sizeof(wheel_torques_));
        std::memset(magnetorquer_, 0, sizeof(magnetorquer_));
    }

    // Clamp actuator commands to limits
    for (int i = 0; i < 3; ++i) {
        // clamp wheel torques to ±max_torque_
        if (wheel_torques_[i] > max_torque_) wheel_torques_[i] = max_torque_;
        if (wheel_torques_[i] < -max_torque_) wheel_torques_[i] = -max_torque_;

        // clamp magnetorquer to [-1, 1]
        if (magnetorquer_[i] > 1.0f) magnetorquer_[i] = 1.0f;
        if (magnetorquer_[i] < -1.0f) magnetorquer_[i] = -1.0f;
    }

    control_cycles_++;
    updateHealthMonitoring();
}

void Microcontroller::getActuatorCommands(ActuatorCommands& commands) {
    for (int i = 0; i < 3; ++i) {
        commands.wheel_torques[i] = wheel_torques_[i];
        commands.magnetorquer[i] = magnetorquer_[i];
    }
    commands.timestamp = control_cycles_;
}

void Microcontroller::setControlMode(int mode) {
    if (mode >= 0 && mode <= 3) {
        control_mode_ = mode;
        std::cout << "Control mode set to: " << mode << std::endl;
    }
}

void Microcontroller::setFaultThreshold(float threshold) {
    if (threshold > 0.0f) {
        fault_threshold_ = threshold;
    }
}

void Microcontroller::performFaultDetection() {
    fault_flags_ = 0;

    for (int i = 0; i < 3; ++i) {
        if (std::abs(gyro_rates_[i]) > fault_threshold_) {
            fault_flags_ |= (1 << i);
        }
    }

    bool sensor_fault = false;
    for (int i = 0; i < 3; ++i) {
        if (std::isnan(gyro_rates_[i]) || std::isinf(gyro_rates_[i]) ||
            std::isnan(magnetometer_[i]) || std::isinf(magnetometer_[i])) {
            sensor_fault = true;
            break;
        }
    }

    if (sensor_fault) {
        fault_flags_ |= 0x08;
    }

    if (fault_flags_ != 0) {
        fault_count_++;

        if (fault_count_ > 3 && control_mode_ != 0) {
            control_mode_ = 0;
            std::cout << "Multiple faults detected. Entering safe mode." << std::endl;
        }
    } else {
        fault_count_ = 0;
    }
}

void Microcontroller::updateHealthMonitoring() {
    // increment uptime every 10 control cycles, but not at cycle 0
    if (control_cycles_ != 0 && (control_cycles_ % 10 == 0)) {
        uptime_seconds_++;
    }
}
