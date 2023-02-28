//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef i2c_HG
#define i2c_HG
/* System include statements */

/* Silicon Labs include statements */
#include "em_i2c.h"
#include "em_cmu.h"
#include "sleep_routines.h"
#include "scheduler.h"

//***********************************************************************************
// global variables
//***********************************************************************************
#define I2C_EM_BLOCK   EM2


#define READ_OP  1
#define WRITE_OP 0

typedef enum {
  init_write,
  write_data,
  init_read,
  read_data,
  rec_data,
  end_process
}DEFINED_STATES;



typedef struct {
  bool      enable;       // enable for I2C communication
  bool      master;       // bool for master vs slave
  uint32_t    refFreq;   // i2c ref clock for bus frequency setup
  uint32_t  freq; //max i2c bus freq to use
  I2C_ClockHLR_TypeDef  clhr; //clock low/high ratio control.

  bool scl_pin_en; // enable / disable for pin scl
  bool sda_pin_en; // enable / disable for pin sda

  uint32_t scl_Location;//SCLLOC value route
  uint32_t sda_Location;//SDALOC value route

  bool irq_ack_en;
  bool rxdata_irq_en;
  bool irq_stop_en;

} I2C_OPEN_STRUCT;


typedef struct {
  I2C_TypeDef *i2cx;
    bool busy;

    uint32_t rwrite;

    uint32_t peripheral_address;
    uint32_t register_address;
    uint32_t *data;
    uint32_t bytes_per_transfer;
    uint32_t i2c_callback;
    DEFINED_STATES current_state;
} I2C_STATE_MACHINE;


//***********************************************************************************
// function prototypes
//***********************************************************************************
void i2c_start(I2C_TypeDef *i2c, uint32_t dev_address, uint32_t mode, uint32_t *data, uint32_t bytes_per_transfer, uint32_t reg_address, uint32_t callback);

void i2c_open(I2C_TypeDef *address, I2C_OPEN_STRUCT *i2c_setup);

void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);

bool busy_state(I2C_TypeDef *i2c);

//static functions, not available outside of i2c.c
//void i2c_ack_sm(I2C_STATE_MACHINE *i2c_ackSM);
//void i2c_receive_sm(I2C_STATE_MACHINE *i2c_ackSM);
//void i2c_msstop_sm(I2C_STATE_MACHINE *i2c_ackSM);
#endif
