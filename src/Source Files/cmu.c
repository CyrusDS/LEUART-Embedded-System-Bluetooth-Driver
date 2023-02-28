/**
 * @file cmu.c
 * @author Cyrus Sowdaey
 * @date 9/26/2021
 * @brief Clock Configuration file.
 *Responsible for cmu_open function that will configure oscillators
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *Configure oscillators so that we are working with the minimal power requirements.
 *
 * @details
 *Enables LE clock while disabling non needed clocks to save power, such as the low freq oscillator and high freq periph clock (HFPER).
 *This file and function only deals with enabling and disabling clocks on the efm, as well as selecting and setting proper clock tree routing.
 * @note
 *Does not have any return, simply a setup function.
 *
 ******************************************************************************/

void cmu_open(void){


    CMU_ClockEnable(cmuClock_HFPER, true);


    // By default, LFRCO is enabled, disable the LFRCO oscillator
    // Disable the LFRCO oscillator
    // What is the enumeration required for LFRCO?
    // It can be found in the online HAL documentation
    CMU_OscillatorEnable(cmuOsc_LFRCO  , false, false);

    // Disable the LFXO oscillator
    // What is the enumeration required for LFXO?
    // It can be found in the online HAL documentation
   CMU_OscillatorEnable(cmuOsc_LFXO  , true, true);

    // No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H1

    // Route LF clock to the LF clock tree
    // What is the enumeration required to placed the ULFRCO onto the proper clock branch?
    // It can be found in the online HAL documentation
   // CMU_ClockSelectSet(cmuClock_LFA , cmuSelect_ULFRCO);    // routing ULFRCO to proper Low Freq clock tree
   CMU_ClockSelectSet(cmuClock_LFA , cmuSelect_ULFRCO);

    // What is the proper enumeration to enable the clock tree onto the LE clock branches?
    // It can be found in the Assignment 2 documentation
    CMU_ClockEnable(cmuClock_CORELE, true); // CORELE is low energy clocks

    CMU_ClockEnable(cmuClock_LEUART0 , true);
    CMU_ClockSelectSet(cmuClock_LFB , cmuSelect_LFXO);

}

