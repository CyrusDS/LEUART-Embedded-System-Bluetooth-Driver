/**
 * @file i2c.c
 * @author Cyrus Sowdaey
 * @date 10/17/2021
 * @brief i2c setup file
 *Responsible for setup of i2c communication and state machine logic
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "i2c.h"

//***********************************************************************************
// Private Variables
//***********************************************************************************
static I2C_STATE_MACHINE i2c0_statemachine_vars, i2c1_statemachine_vars;

//***********************************************************************************
// Private functions
//***********************************************************************************

static void i2c_bus_reset(I2C_TypeDef *i2c);
static void i2c_ack_sm(I2C_STATE_MACHINE *i2c_ackSM);
static void i2c_receive_sm(I2C_STATE_MACHINE *i2c_ackSM);
static void i2c_msstop_sm(I2C_STATE_MACHINE *i2c_ackSM);

/***************************************************************************//**
 * @brief
 * ACK interrupt state machine. Handles ACK interrupts.
 *
 * @details
 * Function is called by i2c IRQ handler when ACK flag is triggered.
 * Upon receiving ACK interrupt: does one of two options depending on the possible cases.
 * init_write will cause the peripheral to initialize a specified register for data to be sent to.
 * write_data will read data from the device and write it to specified register.
 * It will continually go through cases after the previous is completed.
 *
 * @note
 *This function should only do one of two things, other cases should not necessarily occur and as such, the default case has an EFM_ASSERT(false)
 *
 *@param[in] i2c_ackSM
 *Input state machine structure whose variables are to be checked.
 ******************************************************************************/
static void i2c_ack_sm(I2C_STATE_MACHINE *i2c_ackSM){
  switch (i2c_ackSM->current_state){
    case init_write:
      i2c_ackSM->i2cx->TXDATA = i2c_ackSM->register_address;
      if(i2c_ackSM->rwrite == READ_OP) {
      i2c_ackSM->current_state = write_data;
      }
      else if(i2c_ackSM->rwrite == WRITE_OP) {
      i2c_ackSM->current_state = read_data;
      }
      break;

    case write_data:
      i2c_ackSM->i2cx->CMD = I2C_CMD_START;
      i2c_ackSM->i2cx->TXDATA = (i2c_ackSM->peripheral_address << 1) | READ_OP ;
      i2c_ackSM->current_state = init_read;
      break;

    case init_read:
      break;

    case read_data:
      i2c_ackSM->bytes_per_transfer--;
      i2c_ackSM->i2cx->TXDATA = (*(i2c_ackSM->data) >> (8*i2c_ackSM->bytes_per_transfer)) & 0xFF;
      if(i2c_ackSM->bytes_per_transfer == 0) {
          i2c_ackSM->i2cx->CMD = I2C_CMD_STOP;
          i2c_ackSM->current_state = rec_data;
          break;
      }
      break;
     case rec_data:
       break;

    case end_process:
      break;

    default:
      EFM_ASSERT(false);
      break;
  }
}

/***************************************************************************//**
 * @brief
 * RXDATAV interrupt state machine. Handles RXDATAV interrupts.
 *
 * @details
 * Similarly to i2c_ack_sm(), this function is only called when the RXDATAV flag is raised and handled by an IRQhandler.
 * Depending on if more data than expected is sent by the peripheral, state machine will perform different operations.
 * Will continually loop until correct number of bytes are read, as sent by peripheral. Will send a stop and NACK if successful, and will increment state machine.
 *
 *
 * @note
 * This function should continually loop and increment depending on state of read operations from the peripheral.
 *
 * @param[in] i2c_ackSM
 *Input state machine structure whose variables are to be checked.
 ******************************************************************************/
static void i2c_receive_sm(I2C_STATE_MACHINE *i2c_ackSM){
  switch (i2c_ackSM->current_state){
      case init_write:
        break;

      case write_data:
        break;

      case init_read:
            i2c_ackSM->bytes_per_transfer = (i2c_ackSM->bytes_per_transfer - 1);
            *(i2c_ackSM->data) &= ~(0xff << (8*i2c_ackSM->bytes_per_transfer));
            *(i2c_ackSM->data) |= i2c_ackSM->i2cx->RXDATA << (8*i2c_ackSM->bytes_per_transfer);
            if(i2c_ackSM->bytes_per_transfer != 0){
                i2c_ackSM->i2cx->CMD = I2C_CMD_ACK;
                break;

            }else{
                i2c_ackSM->i2cx->CMD = I2C_CMD_NACK;
                i2c_ackSM->i2cx->CMD = I2C_CMD_STOP;
                i2c_ackSM->current_state = rec_data;
                break;

            }
      case read_data:

      case rec_data:

      case end_process:

      default:
        EFM_ASSERT(false);
        break;
    }

}


/***************************************************************************//**
 * @brief
 * MSTOP interrupt state machine. Handles MSTOP interrupts.
 *
 * @details
 * This function is only called by the IRQ handler observing an MSTOP interrupt flag being raised.
 * Upon observing a stop condition has been sent, and thus i2c communication is done, energy modes can be changed.
 *Schedules an event to send data, as the i2c_callback.
 *
 *
 * @note
 *Current state must be in receive_data, as an MSTOP condition cannot occur outside of this (or at least should not)
 *
 *@param[in] i2c_ackSM
 *Input state machine structure whose variables are to be checked.
 ******************************************************************************/
static void i2c_msstop_sm(I2C_STATE_MACHINE *i2c_ackSM){
  switch (i2c_ackSM->current_state){
        case init_write:
          EFM_ASSERT(false);
          break;

        case write_data:
          EFM_ASSERT(false);
          break;

        case init_read:
          EFM_ASSERT(false);
          break;

        case read_data:

        case rec_data:
              sleep_unblock_mode(I2C_EM_BLOCK);
              i2c_ackSM->busy = true;

              i2c_ackSM->current_state = init_write;
              add_scheduled_event(i2c_ackSM->i2c_callback);
          break;

        case end_process:

        default:
          EFM_ASSERT(false);
          break;
      }
}

//***********************************************************************************
// Global functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 * Configures and sets up I2C communication and peripherals.
 *
 * @details
 * Configures peripheral clocks for either I2C0 or I2C1. Additionally, routes i2c peripherals and enables correct interrupts.
 *
 * @note
 * Configuration function, called in app.c in order to setup i2c operation.
 *
 * @param[in] address
 * Address of i2c peripheral to be configured.
 *
 * @param[in] i2c_setup
 * Contains info for CLHR ratio, freq, master bit, ref bit, and enable. Additionally has routing locations.
 ******************************************************************************/
void i2c_open(I2C_TypeDef *address, I2C_OPEN_STRUCT *i2c_setup){

  if(address == I2C0){//set up clocks
      CMU_ClockEnable(cmuClock_I2C0, true);
      i2c0_statemachine_vars.busy = true;
  }
  if(address == I2C1){//set up clocks
      CMU_ClockEnable(cmuClock_I2C1, true);
      i2c1_statemachine_vars.busy = true;
    }


  if ((address->IF & 0x01) == 0) {//verify clock is working
      address->IFS = 0x01;
      EFM_ASSERT(address->IF & 0x01);
      address->IFC = 0x01;
  } else {
      address->IFC = 0x01;
    EFM_ASSERT(!(address->IF & 0x01));
  }

  I2C_Init_TypeDef i2c_local_vals;

  i2c_local_vals.clhr = i2c_setup->clhr;
  i2c_local_vals.freq = i2c_setup->freq;
  i2c_local_vals.master = i2c_setup->master;
  i2c_local_vals.refFreq = i2c_setup->refFreq;
  i2c_local_vals.enable = i2c_setup->enable;

  I2C_Init(address, &i2c_local_vals);


  address->ROUTELOC0 = i2c_setup->scl_Location | i2c_setup->sda_Location;

  address->ROUTEPEN = (i2c_setup->scl_pin_en * I2C_ROUTEPEN_SCLPEN ) | (i2c_setup->sda_pin_en * I2C_ROUTEPEN_SDAPEN);

  address->IEN |= (I2C_IEN_ACK * i2c_setup->irq_ack_en);
  address->IEN |= (I2C_IEN_RXDATAV * i2c_setup->rxdata_irq_en);
  address->IEN |= (I2C_IEN_MSTOP * i2c_setup->irq_stop_en);

  if(address == I2C0){
      NVIC_EnableIRQ(I2C0_IRQn);
  }
  if(address == I2C1){
      NVIC_EnableIRQ(I2C1_IRQn);
    }


  i2c_bus_reset(address);

}

/***************************************************************************//**
 * @brief
 * Performs either a read or a write operation through i2c.
 *
 * @details
 * Depending on input arguments, will configure I2C0 or I2C1 interrupts and peripheral.
 *
 * @note
 * Reads or writes to a peripheral based on inputs.
 *
 * @param[in] i2c
 * Pointer to i2c peripheral that is being interacted with
 *
 * @param[in] dev_address
 * Address of the peripheral being communicated with master device.
 *
 * @param[in] mode
 * Write or Read mode, determines actions of i2c_start
 *
 * @param[in] data
 * Pointer to data which will either be data read from, or data sent to peripheral.
 *
 * @param[in] bytes_per_transfer
 * Expected amount of bytes to be transferred per operation.
 *
 * @param[in] reg_address
 * Address of peripheral whose data will be read from, or written to that register.
 *
 * @param[in] callback
 *Callback to perform upon completing data transfer.
 ******************************************************************************/
void i2c_start(I2C_TypeDef *i2c, uint32_t dev_address, uint32_t mode, uint32_t *data, uint32_t bytes_per_transfer, uint32_t reg_address, uint32_t callback){

  I2C_STATE_MACHINE *i2c_local;
  if(i2c == I2C0){
      i2c_local = &i2c0_statemachine_vars;
  }else if(i2c == I2C1){
      i2c_local = &i2c1_statemachine_vars;
  }else{
      EFM_ASSERT(false);
  }
  while(!i2c_local->busy);

  EFM_ASSERT((i2c->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE);

  sleep_block_mode(I2C_EM_BLOCK); //block unwanted sleep mode ( > EM2)


  i2c_local->busy = false;

  i2c_local->i2cx = i2c;
  i2c_local->rwrite = mode;
  i2c_local->i2c_callback = callback;
  i2c_local->data = data;
  i2c_local->bytes_per_transfer = bytes_per_transfer;
  i2c_local->current_state = init_write;

  i2c_local->register_address = reg_address;
  i2c_local->peripheral_address = dev_address;
  i2c->CMD = I2C_CMD_START;
  i2c->TXDATA = (dev_address << 1) | WRITE_OP;

}


/***************************************************************************//**
 * @brief
 *Performs a reset on the i2c bus.
 *
 * @details
 *Aborts current state of i2c bus, then clears interrupt flags. Clears both astop and start bits to reset all conditions.
 * @note
 * Called in app.c in order to configure i2c and clear any previous conditions, performing a clean reset.
 * @param[in] i2c
 *Pointer to desired i2c peripheral whose bus is to be reset.
 ******************************************************************************/
static void i2c_bus_reset(I2C_TypeDef *i2c) {
  uint32_t ien_save_state; //declare local int

  i2c->CMD = I2C_CMD_ABORT; //ensure state is available to accept commands.

  ien_save_state = i2c->IEN; //store temporary ien state so we keep in mind from before, after checking satte is available

  i2c->IEN = false; //disable all interrupts while config.
  i2c->IFC = i2c->IF; //set state of IF flag

  i2c->CMD = I2C_CMD_CLEARTX; //clear tx bit
  i2c->CMD = (I2C_CMD_START | I2C_CMD_STOP); //clear start and stop bits in case they got triggered

  while(!(i2c->IF & I2C_IF_MSTOP)); //hold until IF register cleared
  i2c->IFC = i2c->IF; //reset state of IF flag in IFC

  i2c->CMD = I2C_CMD_ABORT; //ensure state is left acceptable by resetting.

  i2c->IEN = ien_save_state; //return state of IEN to initial state.
}


/***************************************************************************//**
 * @brief
 * IRQhandler for I2C0 interrupt peripheral.
 *
 * @details
 * Handles any interrupts triggered by the I2C0 peripheral. Based on which flags have been raised, will perform proper operations in response.
 * @note
 * Handles ACK, RXDATAV, and MSTOP interrupt flags.
 ******************************************************************************/
void I2C0_IRQHandler(void) {
  uint32_t int_flag = I2C0->IF & I2C0->IEN;
  I2C0->IFC = int_flag;

  if (int_flag & I2C_IF_ACK){
      i2c_ack_sm(&i2c0_statemachine_vars);
    //EFM_ASSERT(!(I2C0->IF & I2C_IF_ACK));
  }
  if (int_flag & I2C_IF_RXDATAV){
      i2c_receive_sm(&i2c0_statemachine_vars);
    //EFM_ASSERT(!(I2C0->IF & I2C_IF_RXDATAV));

  }
  if (int_flag & I2C_IF_MSTOP){
      i2c_msstop_sm(&i2c0_statemachine_vars);
    //EFM_ASSERT(!(I2C0->IF & I2C_IF_MSTOP));

  }
}

/***************************************************************************//**
 * @brief
 *IRQhandler for I2C1 interrupt peripheral.
 *
 * @details
 * Handles any interrupts triggered by the I2C1 peripheral. Based on which flags have been raised, will perform proper operations in response.
 *
 * @note
 * Handles ACK, RXDATAV, and MSTOP interrupt flags.
 ******************************************************************************/
void I2C1_IRQHandler(void) {
  uint32_t int_flag = I2C1->IF & I2C1->IEN;
  I2C1->IFC = int_flag;

  if (int_flag & I2C_IF_ACK){
      i2c_ack_sm(&i2c1_statemachine_vars);
    //EFM_ASSERT(!(I2C1->IF & I2C_IF_ACK));
  }
  if (int_flag & I2C_IF_RXDATAV){
      i2c_receive_sm(&i2c1_statemachine_vars);
    //EFM_ASSERT(!(I2C1->IF & I2C_IF_RXDATAV));

  }
  if (int_flag & I2C_IF_MSTOP){
      i2c_msstop_sm(&i2c1_statemachine_vars);
    //EFM_ASSERT(!(I2C1->IF & I2C_IF_MSTOP));
  }
}

/***************************************************************************//**
 * @brief
 * Checks busy state of i2c peripheral.
 *
 * @details
 *Determines if i2c periph is available based on input of I2C_Typedef *i2c
 *
 *
 * @note
 *Verifies availability of i2c
 *
 ******************************************************************************/
bool busy_state(I2C_TypeDef *i2c) {
  if(i2c == I2C0) {
      return i2c0_statemachine_vars.busy;
  }
  if(i2c == I2C1) {
      return i2c1_statemachine_vars.busy;
  }
  else {
      return false;
  }
}




