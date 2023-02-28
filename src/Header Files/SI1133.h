//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef Si1133_HG
#define Si1133_HG

/* System include statements */


/* Silicon Labs include statements */
#include "i2c.h"
#include "HW_delay.h"
#include "brd_config.h"

/* The developer's include statements */

//***********************************************************************************
// defined files
//***********************************************************************************
//***********************************************************************************
// global variables
//***********************************************************************************
#define RESPONSE0_REG 0x11
#define INPUT0_REG 0x0A
#define COMMAND_REG 0x0B

#define HOSTOUT0_REG 0x13
#define HOSTOUT1_REG 0x14

#define CHAN_LIST 0x01
#define ADCCONFIG0 0x02

#define PARAMTABLE 0b10000000
#define CHANNEL0_ACTIVE 0b000001
#define FORCE 0x11

#define WRITE_WHITE 0b01011


#define RESET_CMD_CTR 0x00
#define NULL_CB 0x00
#define PART_ID_REGISTER 0x00
#define TimerDelay 25

//***********************************************************************************
// function prototypes
//***********************************************************************************
void Si1133_i2c_open();
void Si1133_read(uint32_t bytes_per_transfer, uint32_t register_address, uint32_t i2c_callback);
void Si1133_force();
void SI1133_request_result();

uint32_t Si1133_read_result();

void Si1133_write(uint32_t bytes_per_transfer, uint32_t register_address, uint32_t i2c_callback);

#endif
