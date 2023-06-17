/*
 * FlowMeasure.c
 *
 *  Created on: 26 sep. 2022
 *      Author: ignaciomendizabal
 */


/* ----- INCLUDE SECTION ----- */
#include <FlowMeter.h>
#include <string.h>
#include <msp430.h>
#include "ussSwLib.h"
#include "utils.h"
#include <IQmathlib.h>
#include <stdbool.h>
#include <hal_timer_a.h>
#include "PressureSensor.h"
#include <math.h>

/* ----- GLOBAL VARIABLES ----- */

flowMeter_Module flow_meter;

extern int16_t gUSSBinaryPattern[USS_BINARY_ARRAY_MAX_SIZE];
#ifdef USS_APP_RESONATOR_CALIBRATE_ENABLE
uint16_t resCalibcount;
#endif
#ifdef USS_APP_AGC_CALIBRATE_ENABLE
uint16_t agcCalibcount;
#endif
#ifdef USS_APP_DC_OFFSET_CANCELLATION_ENABLE
uint16_t dcOffsetEstcount;
#endif

/* Based on flowcharts from USSlib user guide ("Code examples" section, figures 6-7)
 * Library initialization is done in three steps:
 *      1. Initialize the library
 *      2. Generate binary patterns based on pulse generation type (single/dual tone)
 *      3. Initialize the algorithm for AToF calculation (in this case, HillbertWide)
 * Any of these steps could produce an error, so it is important to handle these.
 */
USS_message_code flowMeter_setup(){
    USS_message_code exit_status = USS_message_code_no_error;

#ifdef USS_APP_RESONATOR_CALIBRATE_ENABLE
    resCalibcount = 0;
#endif
#ifdef USS_APP_AGC_CALIBRATE_ENABLE
    agcCalibcount = 0;
#endif
#ifdef USS_APP_DC_OFFSET_CANCELLATION_ENABLE
    dcOffsetEstcount = 0;
#endif


    // 1: Initialize USS library with default parameters
    // This function calls all the functions on the calibration module of the library (configures SDHS, HSPLL, UUPS, etc.)
    exit_status = USS_configureUltrasonicMeasurement(&gUssSWConfig);

    // Verify no errors ocurred during initialization:
    if (exit_status != USS_message_code_no_error){
        //TODO: Handle error
        __no_operation();
        return exit_status;
    }

    // Initialize the binary pattern buffer to 0s
    memset(gUSSBinaryPattern,0,sizeof(gUSSBinaryPattern));

#if defined(USS_GAS_CONFIGURATION) //USE PREPROCESOR CONDITIONALS TO SAVE MEMORY
    // 2. Generate binary patterns. We'll get the pulse generation type from the user config files (i.e: USS_userConfig.h)
    if(USS_measurement_pulse_generation_mode_multi_tone ==
                gUssSWConfig.measurementConfig->pulseConfig->pulseGenMode){

        exit_status = USS_generateMultiToneBinaryPattern(&gUssSWConfig); // For multi-tone (sweep)
        }
    else{
        exit_status = USS_generateMonoDualToneBinaryPattern(&gUssSWConfig); // For dual tone
        }

    if (USS_message_code_no_error != exit_status){
        //TODO: Handle error
        __no_operation();
        return exit_status;
        }

#endif

    // 3. Initialize algorithms:
    gUssSWConfig.algorithmsConfig->isInitAlgorithms = false;

    exit_status = USS_initAlgorithms(&gUssSWConfig);
    if (USS_message_code_no_error != exit_status)
    {
        // TODO: Handle error
        __no_operation();
        return exit_status;

    }

    // Set the background timer period to 1 second
    USS_configAppTimerPeriod(&gUssSWConfig, gUssSWConfig.systemConfig->measurementPeriod);

    USSSWLIB_USS_interrupt_status = 0;


#ifdef USS_PRECHARGE_ENABLE // Perform some captures to charge capacitors (skipped if USS_PRECHARGE_ENABLE undefined)
    {
    int i;

    for (i=0; i < 20; i++){
        exit_status = USS_startLowPowerUltrasonicCapture(&gUssSWConfig);//USS_startUltrasonicMeasurement(&gUssSWConfig,USS_capture_power_mode_active);
        // Invalid capture must report error
        if (exit_status != USS_message_code_no_error){
            USS_resetUSSModule(&gUssSWConfig, true);
            }
        }
    }
#endif

    return exit_status;
}

_iq16 flowMeter_measureDToF(){
    USS_Algorithms_Results_fixed_point results; //results in fixed point format
    USS_message_code exit_status = USS_message_code_no_error;

#ifdef USS_APP_RESONATOR_CALIBRATE_ENABLE
    USS_calibration_hspll_results pllCalibTestResults;
#endif

    // 1. Start ultrasonic capture: generate pulse for transceiver and capture the waveform on receiver:


    exit_status = USS_startLowPowerUltrasonicCapture(&gUssSWConfig);
    if (exit_status != USS_message_code_no_error){

        if (USSSWLIB_USS_interrupt_status & USS_HSPLL_PLL_UNLOCK_INTERRUPT){
            USS_resetUSSModule(&gUssSWConfig, true);
        }
        //TODO: handle error
        __no_operation();
        return 0;
    }

#ifdef USS_APP_DC_OFFSET_CANCELLATION_ENABLE
                // Calibrate the gain amplifier.
                // This routine will update the agcConstant with optimal gain
                if (USS_App_userConfig.u16DCOffsetEstimateInterval != 0)
                {
                    if (++dcOffsetEstcount >= USS_App_userConfig.u16DCOffsetEstimateInterval)
                    {
                        dcOffsetEstcount = 0;
                        // Perform DC offset estimation using measurement sequence going
                        // to LPM3 between UPS and DNS capture
                        exit_status = USS_estimateDCoffset(&gUssSWConfig,
                                                USS_dcOffEst_Calc_Mode_use_existing_UPS_DNS_capture );
                    }
                }
#endif

    // 2. Run algorithms on captured waveform to get measurement results (in fixed point):
    exit_status = USS_runAlgorithmsFixedPoint(&gUssSWConfig, &results);
    if ( (exit_status != USS_message_code_valid_results)
            && (exit_status != USS_message_code_algorithm_captures_accumulated) ){


        //TODO: handle error
        return 0;

    }

    /*DEPRECATED: 3. Parse obtained results to floating point:

    exit_status = USS_getResultsInFloat(&results,&results_f);
    if (exit_status != USS_message_code_no_error){
        //TODO: handle error
        __no_operation();

    }*/

// ------ ADITIONAL CALIBRATION  ------
// HSPLL Frequency verification test: compensate AToF & DToF calculation errors due to frequency drift:
#ifdef USS_APP_RESONATOR_CALIBRATE_ENABLE
        // Calibrate the Resonator against the LFXT and obtain the correction
        // term relative to the expected value obtained during Initialization
        // Based on USSlib User guide, "Library Calibration Routines" section
        if (USS_App_userConfig.u16ResonatorCalibrateInterval != 0)
        {
            if (++resCalibcount >= USS_App_userConfig.u16ResonatorCalibrateInterval)
            {
                resCalibcount = 0;
                __disable_interrupt();
                code = USS_verifyHSPLLFrequency(&gUssSWConfig,&pllCalibTestResults);
                __enable_interrupt();
                if (code == USS_message_code_no_error)
                {
                    // Update relative error if the HSPLL frequency is within range
                    gUssSWConfig.algorithmsConfig->clockRelativeError =
                            (_IQ27div(pllCalibTestResults.difference,
                                      pllCalibTestResults.expectedResult));
                    USS_updateClockRelativeError(&gUssSWConfig);
                }
                else
                {
                    // Report error if the HSPLL frequency is outside the expected range
                    if (code == USS_message_code_HSPLL_pll_unlock_error)
                    {
                        // Reset USS if a PLL unlock error is detected
                        USS_resetUSSModule(&gUssSWConfig, true);
                    }
                    //TODO: handle error
                    return 0;

                }
            }
        }
#endif

#ifdef USS_APP_AGC_CALIBRATE_ENABLE
        // Calibrate the gain amplifier.
        // This routine will update the agcConstant with optimal gain.
        if (USS_App_userConfig.u16AGCCalibrateInterval != 0)
        {
            if (++agcCalibcount >= USS_App_userConfig.u16AGCCalibrateInterval)
            {
                agcCalibcount = 0;
                exit_status = USS_calibrateSignalGain(&gUssSWConfig);
            }
        }
#endif

#ifdef __WATCHDOG_ENABLE__
        hal_system_WatchdogFeed();
#endif

        /* results are expressed in l/h, we have to convert them to m^3/h
         *  that is: new_flow = measured_flow/1000  ~  measured_flow/1024
         *  1024 = 64 * 16 so new_flow ~ (measured_flow/64)/16
         *  this makes calculations way faster
         *  also get absolute value (we don't care about the direction of the flow, only the modulus
         */
    _iq16 DToF = results.iq16VolumeFlowRate;
    return DToF;
}

_iq16 flowMeter_calculateVolumeFlowRate(_iq16 DToF){
    // Calculate volume flow rate multiplying the differential time of flight by the volume scale factor:
    _iq16 VolumeFlowRate = _IQ16mpy(DToF,VSF);
    return VolumeFlowRate;
}

float flowMeter_calculateMassFlowRate(_iq16 DToF){
    // Calculate mass flow rate using the calibration curve:
    return (1.7175 * _IQ16toF(DToF) + 81.4780) * _IQ16toF(DToF);
}

void flowMeter_update(){
    // Main loop of the flow meter module
    // Measures mass flow rate, updates totalizer & handles overflows (WIP)

    // first, get the differential time of flight:
    flow_meter.last_DToF = flowMeter_measureDToF();

    //flow_meter.last_DToF = 524590; //8 en punto fijo

    // calculate volume flow rate:
    _iq16 flow_rate = flowMeter_calculateVolumeFlowRate(flow_meter.last_DToF);
    flow_meter.last_Volume_Flow_Rate = flow_rate;

    // calculate mass flow rate:
    float mass_flow_rate = flowMeter_calculateMassFlowRate(flow_meter.last_DToF);
    flow_meter.last_Mass_Flow_Rate = mass_flow_rate;

    if ( (_IQ16toF(flow_meter.last_DToF) < 13) && (_IQ16toF(flow_meter.last_DToF) > 1.5) ){

        //hour to seconds conversion
        float float_mass_flow_rate_per_second = mass_flow_rate / 3600;
        flow_meter.measurement_Count++;

        //add flow measurement to totalizer;
        flow_meter.totalizer += MEASURE_WAKE_UP_TIME * float_mass_flow_rate_per_second;
    }
}

inline float flowMeter_getTotalizer(){
    return flow_meter.totalizer;
}

inline float flowMeter_getAverageMassFlowRate(){
    float AverageMassFlowRate = (3600 * flow_meter.totalizer) / (MEASURE_WAKE_UP_TIME * flow_meter.measurement_Count);
    return AverageMassFlowRate;
}

inline float flowMeter_getVolumeFlowRate(){
    return _IQ16toF(flow_meter.last_Volume_Flow_Rate);
}

inline float flowMeter_getMassFlowRate(){
    return flow_meter.last_Mass_Flow_Rate;
}

void flowMeter_setMeasurementTimeInterval(uint16_t interval){
    hal_timer_a_setWakeUptimerPeriod(interval*512);
    flow_meter.measure_Time_Interval_Seconds = interval;
}

void flowMeter_reset(){
    flow_meter.totalizer = 0;
    flow_meter.measurement_Count = 0;
    flow_meter.last_Mass_Flow_Rate = 0;
    flow_meter.last_Volume_Flow_Rate = 0;
    flow_meter.last_DToF = 0;
}

