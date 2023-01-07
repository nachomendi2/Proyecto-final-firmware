/*
 * Memory.c
 *
 *  Created on: 26 sep. 2022
 *      Author: ignaciomendizabal
 */

#include "FlowMeter.h"
#include "ValveControl.h"
#include "nvs.h"
#include "stdbool.h"

extern ValveControl_Module valve;
extern flowMeter_Module flow_meter;

// create NVS data handles for both modules
nvs_data_handle nvsHandleFlowMeter;
nvs_data_handle nvsHandleValve;

// Allocate memory in FRAM for flowMeter & ValveControl modules backups via NVS containers
#if defined(__TI_COMPILER_VERSION__)
#pragma PERSISTENT(nvsStorageValveModule)
#elif defined(__IAR_SYSTEMS_ICC__)
__persistent
#endif
uint8_t nvsStorageValveModule[NVS_DATA_STORAGE_SIZE(sizeof(valve))] = {0};

#if defined(__TI_COMPILER_VERSION__)
#pragma PERSISTENT(nvsStorageFlowMeterModule)
#elif defined(__IAR_SYSTEMS_ICC__)
__persistent
#endif
uint8_t nvsStorageFlowMeterModule[NVS_DATA_STORAGE_SIZE(sizeof(flow_meter))] = {0};

bool Memory_loadBackup(){

    bool return_value = true;
    uint16_t operation_status;

    // Check integrity of NVS container and initialize if required
    nvsHandleFlowMeter = nvs_data_init(nvsStorageFlowMeterModule, sizeof(flow_meter));

    // Restore flow meter module data:
    operation_status = nvs_data_restore(nvsHandleFlowMeter, &flow_meter);

    // Verify no errors ocurred
    switch (operation_status) {
        case NVS_OK: break;
        case NVS_EMPTY:
            // If no previous backup found, load default values to flow_meter module:
            // NOTE: maybe some of these can be avoided, as restore() should load zeroes
            // into the struct in this case, but for security reasons initialize then explictly:
            flow_meter.totalizer = 0;
            flow_meter.measurement_Count = 0;
            flow_meter.pressure = _IQ16(1.898);
            flow_meter.temperature = _IQ16(15.F);
            break;
        default:
            /*
             * Status should never be NVS_NOK or NVS_CRC_ERROR, potential reasons:
             *     1. nvsStorage not initialized
             *     2. nvsStorage got corrupted by other task (buffer overflow?)
             */
            return_value = false;
        }

    nvsHandleValve = nvs_data_init(nvsStorageValveModule, sizeof(valve));

    // Repeat with valve module
    operation_status = nvs_data_restore(nvsHandleValve, &valve);
    switch (operation_status) {
            case NVS_OK: break;
            case NVS_EMPTY:
                // If no previous backup found, set valve state to default init state:
                valve.state = VALVE_INIT_STATE;
                valve.command_Pulse_Delay.restartAfterCompletion = false;
                break;
            default: return_value = false;
            }

    return return_value;
}

nvs_status test;
bool Memory_backupData(){
    uint16_t operation_status = nvs_data_commit(nvsHandleFlowMeter, &flow_meter);
    test = operation_status;
    if (operation_status != NVS_OK){
        return false;
    }

    operation_status = nvs_data_commit(nvsHandleValve, &valve);
    if (operation_status != NVS_OK){
        return false;
    }

    return true;
}
