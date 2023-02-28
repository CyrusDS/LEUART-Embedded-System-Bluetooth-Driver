//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef APP_HG
#define APP_HG

/* System include statements */


/* Silicon Labs include statements */

#include "em_cmu.h"
#include "em_assert.h"
#include <stdio.h>


/* The developer's include statements */
#include "cmu.h"
#include "gpio.h"
#include "letimer.h"
#include "brd_config.h"

#include "scheduler.h"
#include "LEDs_thunderboard.h"
#include "sleep_routines.h"
#include "Si1133.h"

#include "HW_delay.h"
#include "ble.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define SYSTEM_BLOCK_EM EM3


#define DELAYTIME 2000
#define   PWM_PER         2.0   // PWM period in seconds
#define   PWM_ACT_PER     0.002  // PWM active period in seconds


#define   EXPECTED_DATA  51 //Part ID to be returned from a read. Not needed for lab 5
#define   READ_DATA_B          1 //Bytes to be read from SI1133

#define LETIMER0_COMP0_CB 0x00000001 //0b0001
#define LETIMER0_COMP1_CB 0x00000002 //0b0010
#define LETIMER0_UF_CB 0x00000004 //0b0100

#define SI1133_CB 0x00000008   //0b1000
#define EXPECTED_READ 20 //Lab 5 sensor value to be read

#define BOOT_UP_CB 0x00000010
#define TX_CALLBACK 0x00000020
#define RX_CALLBACK 0x00000040
#define BLE_TX_DONE_CB 0x00000080


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void app_peripheral_setup(void);

void scheduled_letimer0_uf_cb(void);
void scheduled_letimer0_comp0_cb(void);
void scheduled_letimer0_comp1_cb(void);

void scheduled_si1133_read_cb(void);

void scheduled_boot_up_cb(void);

void scheduled_BLE_TX_DONE_CB(void);


void BLE_TX_cb(void);
void BLE_RX_cb(void);

#endif
