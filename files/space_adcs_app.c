#include "cfe.h"
#include "space_adcs_app.h"
#include <string.h>

// External C wrapper calls to C++ microcontroller
extern void Microcontroller_Init(void);
extern void Microcontroller_ProcessSensor(const SensorData_t* sensor);
extern void Microcontroller_GetActuatorCommands(ActuatorCommands_t* commands);

// Global app data
typedef struct {
    CFE_SB_PipeId_t SensorPipe;
    uint32 RunStatus;
} SpaceADCS_Data_t;

SpaceADCS_Data_t SpaceADCS_Data;

void SpaceADCS_AppMain(void) {
    CFE_Status_t Status;

    // Initialization event (optional, can enable later)
    // CFE_EVS_SendEvent(1, CFE_EVS_EventType_INFORMATION, "=== SPACE ADCS APP STARTING ===");

    // Register for Event Services
    Status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("Space ADCS: Error registering EVS, RC = 0x%08lX\n", (unsigned long)Status);
        return;
    }

    // Create Software Bus Pipe for sensor data
    Status = CFE_SB_CreatePipe(&SpaceADCS_Data.SensorPipe, 16, "ADCS_SENSOR_PIPE");
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(3, CFE_EVS_EventType_ERROR,
                          "SPACE ADCS: Error creating pipe, RC = 0x%08lX",
                          (unsigned long)Status);
        return;
    }

    // Setup Sensor Message ID
    CFE_SB_MsgId_t SensorMsgId = CFE_SB_ValueToMsgId(0x0800);

    // FAULT: Sensor subscription skipped
    // Uncomment to receive sensor data:
    // Status = CFE_SB_Subscribe(SensorMsgId, SpaceADCS_Data.SensorPipe);
    // if (Status != CFE_SUCCESS) {
    //     CFE_EVS_SendEvent(4, CFE_EVS_EventType_ERROR,
    //                       "SPACE ADCS: Failed to subscribe to sensor, RC = 0x%08lX",
    //                       (unsigned long)Status);
    // }

    // Initialize microcontroller
    Microcontroller_Init();

    // Set application run status
    SpaceADCS_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    // Main loop
    while (CFE_ES_RunLoop(&SpaceADCS_Data.RunStatus)) {
        CFE_SB_Buffer_t* SBBufPtr = NULL;

        // Wait for sensor message (blocking)
        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, SpaceADCS_Data.SensorPipe, CFE_SB_PEND_FOREVER);
        if (Status == CFE_SUCCESS && SBBufPtr != NULL) {
            // Cast buffer to sensor data
            SensorData_t* sensor = (SensorData_t*)SBBufPtr;

            // Process sensor data in microcontroller
            Microcontroller_ProcessSensor(sensor);

            // Get actuator commands from microcontroller
            ActuatorCommands_t commands;
            Microcontroller_GetActuatorCommands(&commands);

            // TODO: send actuator commands to actuators (if required)
        } else if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(5, CFE_EVS_EventType_ERROR,
                              "SPACE ADCS: Failed to receive buffer, RC = 0x%08lX",
                              (unsigned long)Status);
        }
    }

    // Exit application
    CFE_ES_ExitApp(SpaceADCS_Data.RunStatus);
