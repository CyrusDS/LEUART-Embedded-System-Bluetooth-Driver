/**
 * @file Si1133.c
 * @author Cyrus Sowdaey
 * @date 10/17/2021
 * @brief Si1133 Setup file
 *Fill in
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "Si1133.h"

//***********************************************************************************
// defined files
//***********************************************************************************
static uint32_t Si1133_read_data;
uint32_t periph_address = 0x55;

static uint32_t Si1133_write_data;

static void Si1133_configure();

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************

//***********************************************************************************
// private functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * Configures Si133 read operation from the sensor, setting channel0 active.
 * @details
 * Resets counter initially to prevent miswrtes. After that, the RESPONSE0 reg is read
 * and the value is stored in the CMD_CTR_DATA. Specified color is then written to
 *sensor for white light in the INPUT0 register. Further configuration is done
 e first reset the reset cmd ctr to prevent any unwanted bugs. Then we read the
 * response0 register which is what holds the cmd ctr. The value of CHANNEL0 is sent to
 * INPUT0 and the chan list is sent to the command register. RESPONSE0 is read and
 * it is ensured that the CMD CTR Data is incremented for a second time.
 *
 *
 *
 *
 * @note
 * Called by Si1133_i2c_open, has no inputs.
 *
 ******************************************************************************/
static void Si1133_configure() {
  uint32_t CMD_CTR_Data; //declare local private variable for CMD_CTR to keep track

  Si1133_write_data = RESET_CMD_CTR;

  Si1133_write(1, COMMAND_REG, NULL_CB); //send reset to counter through command register
  while(!busy_state(I2C1)); //verify not busy


  Si1133_read(1, RESPONSE0_REG, NULL_CB);

  CMD_CTR_Data = (Si1133_read_data & 0x0F); //obtain cmd count initial (by reading RESPONSE0_REG above

  while(!busy_state(I2C1)); //verify not busy

  Si1133_write_data = WRITE_WHITE; //set white photo diode value
  Si1133_write(1, INPUT0_REG, NULL_CB); //write to INPUT0_REG to set ADCMUX value as white


  while(!busy_state(I2C1));//verify not busy

  Si1133_write_data = PARAMTABLE | ADCCONFIG0; //parameter table write OR'd with ADCCONFIG0
  Si1133_write(1, COMMAND_REG, NULL_CB);//write to command reg what is in Si1133_write_data

  while(!busy_state(I2C1));//verify not busy
  Si1133_read(1, RESPONSE0_REG, NULL_CB); //check state of RESPONSE0_REG

  while(!busy_state(I2C1));//verify not busy


  if((Si1133_read_data & 0x0F) != (CMD_CTR_Data+1)) { //compare to verify CMD_CTR got incremented
      EFM_ASSERT(false);
  }


  Si1133_write_data = CHANNEL0_ACTIVE; //value to write to INPUT0_REG
  Si1133_write(1,INPUT0_REG,NULL_CB); //write to INPUT0_REG to set Channel0 as active Si1133 Channel

  while(!busy_state(I2C1));//verify not busy

  Si1133_write_data = PARAMTABLE | CHAN_LIST; //parameter table write OR'd with CHAN_LISt
  Si1133_write(1, COMMAND_REG, NULL_CB); //write that to command_reg

  while(!busy_state(I2C1));//verify not busy

  Si1133_read(1,RESPONSE0_REG, NULL_CB); //read RESPONSE0_REG to verify command success

  while(!busy_state(I2C1));//verify not busy

  if((Si1133_read_data  & 0x0F) != (CMD_CTR_Data+2)) { //verify CMD_CTR got incremented again
    EFM_ASSERT(false);
  }
  else {
      return; //success! exit si1133_configure function
  }

}

//***********************************************************************************
// global functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 * send force command to Si1133 sensor
 *
 * @details
 * Writes a force command to the command register so that it is executed
 *
 *
 *
 * @note
 *Call occurs in Comp1_Callback
 *
 ******************************************************************************/
void Si1133_force() {
//  Si1133_read(1,RESPONSE0_REG, NULL_CB);
//  while(!busy_state(I2C1));//verify not busy


  Si1133_write_data = FORCE; //set force command to write
  Si1133_write(1,COMMAND_REG, NULL_CB); //write to COMMAND_REG our FORCE command
}
/***************************************************************************//**
 * @brief
 * Requests a read operation from Si1133 sensor
 *
 * @details
 *A total of two bytes should be read from the HOSTOUT0_REG
 *
 *
 *
 * @note
 *Call occurs from UB_Callback
 *
 ******************************************************************************/
void SI1133_request_result(uint32_t LIGHT_CB) {
  Si1133_read(2,HOSTOUT0_REG, LIGHT_CB);
}







/***************************************************************************//**
 * @brief
 *Initializes i2c parameters for specifically the Si1133 peripheral.
 *
 * @details
 *Configures a local I2C_OPEN_STRUCT, si_values and places initial parameters within.
 *
 * @note
 *I2C open function is run with this configuration, after the struct is filled with variables.
 *
 ******************************************************************************/

void Si1133_i2c_open() {
  timer_delay(TimerDelay);
  I2C_OPEN_STRUCT si_values;

  si_values.clhr = i2cClockHLRAsymetric;
  si_values.enable = true;
  si_values.freq = I2C_FREQ_FAST_MAX ;

  si_values.master = true;
  si_values.scl_pin_en = true;
  si_values.sda_pin_en = true;

  si_values.refFreq = 0;

  si_values.scl_Location = I2C_ROUTE_SCL_0;
  si_values.sda_Location = I2C_ROUTE_SDA_0;

  si_values.irq_ack_en  = true;
  si_values.rxdata_irq_en = true;
  si_values.irq_stop_en = true;

  i2c_open(I2C1, &si_values);
  Si1133_configure();
}

/***************************************************************************//**
 * @brief
 * Performs a read operation on SI1133 peripheral.
 *
 * @details
 * Calls i2c_start() with proper parameters to allow data to be read from SI1133
 *
 * @note
 *Called in app.c upon configuration completion.
 *
 * @param[in] bytes_per_transfer
 * Bytes to read in a transfer from peripheral (si1133)
 *
 * @param[in] register_address
 * Register to be read from of peripheral (si1133)
 *
 * @param[in] i2c_callback
 * Callback function to run after read operation.
 *
 ******************************************************************************/
void Si1133_read(uint32_t bytes_per_transfer, uint32_t register_address, uint32_t i2c_callback){
  i2c_start(I2C1, periph_address, READ_OP, &Si1133_read_data, bytes_per_transfer, register_address, i2c_callback);
}





/***************************************************************************//**
 * @brief
 * Performs a write operation on SI1133 peripheral.
 *
 * @details
 * Calls i2c_start() with proper parameters to allow data to be written to SI1133
 *
 * @note
 *Called in app.c upon configuration completion.
 *
 * @param[in] bytes_per_transfer
 * Bytes to write in a transfer to peripheral (si1133)
 *
 * @param[in] register_address
 * Register to be written to of peripheral (si1133)
 *
 * @param[in] i2c_callback
 * Callback function to run after write operation.
 *
 ******************************************************************************/
void Si1133_write(uint32_t bytes_per_transfer, uint32_t register_address, uint32_t i2c_callback){
  i2c_start(I2C1, periph_address, WRITE_OP, &Si1133_write_data, bytes_per_transfer, register_address, i2c_callback);
}




/***************************************************************************//**
 * @brief
 * Returns data placed in Si1133_read_data private variable.
 *
 * @details
 * This function return the read data from a private static struct in order to access the data within another file
 * Called within callback function in app.c upon completion of operation.
 *
 * @note
 *Simple function, does not actively change any data, only returns an integer.
 ******************************************************************************/

uint32_t Si1133_read_result(){
  return Si1133_read_data;
}




