#ifndef _USSSWLIBCOMMONSDHS_H_
#define _USSSWLIBCOMMONSDHS_H_

//#############################################################################
//
//! \file   ussSwLib.h
//!
//! \brief  Contains all USS SW Library enums, structs, macros, function and
//!			global variables defintions
//!         
//
//  Group:          MSP
//  Target Device:  Banshee
//
//  (C) Copyright 2019, Texas Instruments, Inc.
//#############################################################################
// TI Release: USSLib_02_30_00_03 
// Release Date: February 04, 2020
//#############################################################################


//*****************************************************************************
// includes
//*****************************************************************************
#include <libraries/USS/source/common/ussSwLibCommonUSS.h>
#include <USS_Lib_HAL.h>
#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include "ussSwLib.h"

//*****************************************************************************
//! \addtogroup ussSwLib 
//! @{
//*****************************************************************************

#ifdef __cplusplus

extern "C" {
#endif

//*****************************************************************************
// defines
//*****************************************************************************


//*****************************************************************************
// typedefs
//*****************************************************************************


//*****************************************************************************
// globals
//*****************************************************************************


//*****************************************************************************
// the function prototypes
//*****************************************************************************
extern USS_message_code commonValidateSDHSconfigruation(
		USS_SW_Library_configuration *config);

extern inline void commonSDHSRestoreSDHSDTCA(
		USS_SW_Library_configuration *config);

extern inline void commonSDHSConfigModOpt(USS_SW_Library_configuration *config);


#ifdef __cplusplus
}
#endif // extern "C"
//@}  // ingroup

#endif // end of  _USSSWLIBSDHS_H_ definition
