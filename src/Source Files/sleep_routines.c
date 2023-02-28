/**
 * @file sleep_routines.c
 * @author Cyrus Sowdaey
 * @date 10/17/2021
 * @brief sleep routines
 *Responsible for handling sleep routines, sleep routine blocking, unblocking, and setup.
 */

/**************************************************************************
* @file sleep.c
***************************************************************************
* @section License
* <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
***************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
* obligation to support this Software. Silicon Labs is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Silicon Labs will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
**************************************************************************/


//***********************************************************************************
// Include files
//***********************************************************************************
#include "sleep_routines.h"


//private variables
static int lowest_energy_mode[MAX_ENERGY_MODES];

/***************************************************************************//**
 * @brief
 *Enters sleep energy mode - dependent on the first non-zero array element stored in lowest_energy_mode[]
 *
 * @details
 *Will decrease energy mode to sleep, depending on current mode. No changes if already in sleep mode.
 *
 * @note
 *Atomic operations occur if energy mode is not currently in sleep mode.
 *
 ******************************************************************************/
void enter_sleep(void) {
  if (lowest_energy_mode[EM0] > 0) {
    return;
  }

  else if(lowest_energy_mode[EM1] > 0) {
    return;
  }

  else if (lowest_energy_mode[EM2] > 0) {
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();
    EMU_EnterEM1();
    CORE_EXIT_CRITICAL();
    return;
  }

  else if (lowest_energy_mode[EM3] > 0) {\
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();
    EMU_EnterEM2(true);
    CORE_EXIT_CRITICAL();
    return;
  }

  else {
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();
    EMU_EnterEM3(true);
    CORE_EXIT_CRITICAL();
    return;
  }
}

/***************************************************************************//**
 * @brief
 *Returns currently blocked energy modes.
 *
 * @details
 *Runs through lowest_energy_mode[i] until MAX_ENERGY_MODES to determine specific element that is blocked.
 *
 * @note
 *No atomic operations occur, no danger here.
 *
 * @param[out] i
 *Returns the current energy mode that is blocked.
 ******************************************************************************/
uint32_t current_block_energy_mode(void) {
  int i = 0;
  for (i = 0; i < MAX_ENERGY_MODES; i++) {
      if(lowest_energy_mode[i] != 0) {
          return i;
      }
  }
  return (MAX_ENERGY_MODES - 1); //offset by 1 for array

}
/***************************************************************************//**
 * @brief
 *Releases processor from sleep mode.
 *
 * @details
 *The energy mode specified by EM will change the energy state to be no longer active.
 *
 * @note
 *Decrements lowest_energy_mode[EM] by 1. An assert is also used for verification.
 *
 * @param[in] EM
 *input of 0 to 4, determines state not to unblock.
 ******************************************************************************/
void sleep_unblock_mode(uint32_t EM) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  lowest_energy_mode[EM] = lowest_energy_mode[EM] -1 ;

  CORE_EXIT_CRITICAL();
  EFM_ASSERT(lowest_energy_mode[EM] >= 0);
  return;
}
/***************************************************************************//**
 * @brief
 *Prevents gecko from going into an energy state, specified by EM
 *
 * @details
 *Incrents lowest_energy_mode[EM] by 1.
 *
 * @note
 *Atomic operation occurs here, and an assert also exists. Beware of this if issues are found.
 *
 * @param[in] EM
 *input of energy mode to block.
 ******************************************************************************/
void sleep_block_mode(uint32_t EM) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  lowest_energy_mode[EM] = lowest_energy_mode[EM] + 1 ;

  CORE_EXIT_CRITICAL();
  EFM_ASSERT(lowest_energy_mode[EM] < 5);
  return;
}

/***************************************************************************//**
 * @brief
 *Initial setup for energy modes and sleep states
 *
 * @details
 *Sets all elements of lowest_energy_mode[] to 0.
 *
 * @note
 *Atomic operations performed when performing initial setup.
 *
 ******************************************************************************/
void sleep_open(void) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  for (int j=0; j < MAX_ENERGY_MODES; j++) {
      lowest_energy_mode[j] = 0;
  }
  CORE_EXIT_CRITICAL();
  return;
}
