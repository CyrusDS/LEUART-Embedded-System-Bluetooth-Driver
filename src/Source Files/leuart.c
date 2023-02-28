/**
 * @file
 * leuart.c
 * @author
 * Cyrus Sowdaey
 * @date
 * 11/9/21
 * @brief
 * Contains all the functions of the LEUART peripheral
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Library includes
#include <string.h>

//** Silicon Labs include files
#include "em_gpio.h"
#include "em_cmu.h"

//** Developer/user include files
#include "leuart.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
uint32_t	rx_done_evt;
uint32_t	tx_done_evt;
bool		leuart0_tx_busy;

static LEUART_WRITE_SM leuart0_SM; //write
static LEUART_READ_SM leuart0_SM_READ; //read


/***************************************************************************//**
 * @brief LEUART driver
 * @details
 *  This module contains all the functions to support the driver's state
 *  machine to transmit a string of data across the LEUART bus.  There are
 *  additional functions to support the Test Driven Development test that
 *  is used to validate the basic set up of the LEUART peripheral.  The
 *  TDD test for this class assumes that the LEUART is connected to the HM-18
 *  BLE module.  These TDD support functions could be used for any TDD test
 *  to validate the correct setup of the LEUART.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************
static void STARTFRAME_HANDLER(LEUART_READ_SM*leuart0_SM_READ);
static void SIGFRAME_HANDLER(LEUART_READ_SM*leuart0_SM_READ);
static void RXDATAV_HANDLER(LEUART_READ_SM*leuart0_SM_READ);



//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * Configures and sets up LEUART communication.
 * @details
 * Configures LEUART and LEUART routing, as wel as running LEUART_Init and LEUART_Enable functions with the input LEUART_Typedef *leuart struct.
 *
 * @note
 * Local LEUART_Init_Typedef struct is used, as well as an LEUART_Enable_Typedef struct to ensure the local struct is disabled during configuration.
 *
 * @param[in] leuart
 * Address of leuart peripheral to be configured.
 *
 * @param[in] leuart_settings
 * Contains info for the desired configuration of the LEUART peripheral.
 ******************************************************************************/
void leuart_open(LEUART_TypeDef * leuart, LEUART_OPEN_STRUCT *leuart_settings){
  if(leuart == LEUART0) {
        CMU_ClockEnable(cmuClock_LEUART0, true);
    }

    leuart->STARTFRAME = true;

    while(leuart->SYNCBUSY);
    EFM_ASSERT(leuart->STARTFRAME == true);

    leuart->STARTFRAME = false;

    while(leuart->SYNCBUSY);
    EFM_ASSERT(leuart->STARTFRAME == false);

    LEUART_Init_TypeDef leuart_values;
    leuart_values.enable = leuartEnable;
    leuart_values.refFreq = 0;

    leuart_values.baudrate = leuart_settings->baudrate;
    leuart_values.parity = leuart_settings->parity;
    leuart_values.databits = leuart_settings->databits;
    leuart_values.stopbits = leuart_settings->stopbits;

    LEUART_Init(leuart, &leuart_values);

    while(leuart->SYNCBUSY);
    leuart->ROUTELOC0 = leuart_settings->tx_loc | leuart_settings->rx_loc;
    leuart->ROUTEPEN = (leuart_settings->tx_en * LEUART_ROUTEPEN_TXPEN) | (leuart_settings->rx_en * LEUART_ROUTEPEN_RXPEN);

    leuart->CMD |= LEUART_CMD_CLEARRX;
    leuart->CMD |= LEUART_CMD_CLEARTX;
    while(!(leuart->STATUS & LEUART_STATUS_TXENS));
    while(!(leuart->STATUS & LEUART_STATUS_RXENS));

    EFM_ASSERT((leuart->STATUS & LEUART_STATUS_TXENS));
    EFM_ASSERT((leuart->STATUS & LEUART_STATUS_RXENS));

    NVIC_EnableIRQ(LEUART0_IRQn);
    leuart-> IFC |= IFC_CLR;
    leuart->IEN |= LEUART_IEN_STARTF;

    while(leuart->SYNCBUSY);
    leuart->CTRL |= LEUART_CTRL_SFUBRX;

    while(leuart->SYNCBUSY);
    leuart0_SM_READ.leuart_read = leuart;

    leuart0_SM_READ.leuart0_read_cb = BLE_TX_DONE_CB;
    leuart0_SM_READ.leuart0_read_cb = rx_done_evt;


    leuart0_SM_READ.leuart_read->STARTFRAME = STARTF_CHR;
    leuart0_SM_READ.leuart_read->SIGFRAME = SIGF_CHR;

    leuart->CMD |= LEUART_CMD_RXBLOCKEN;

    while(leuart->SYNCBUSY);
    LEUART_Enable(leuart, leuartEnable);

    leuart0_SM_READ.current_read_state = STARTFRAME;
    leuart0_SM_READ.str_length = 0;
    leuart_rx_tdd();
}

/***************************************************************************//**
 * @brief
 * Initializes transmission of a string with specific length
 * @details
 * Blocks sleep mode, sets busy, and passes string to SM struct
 *
 * @param[in] leuart
 * Address of leuart peripheral to be started and written to
 *
 *  @param[in] string
 *Reference to string to be written to peripheral.
 *
 *  @param[in] string_len
 * Length of string to be written to peripheral.
 *
 *   @param[in] leuart_cb
 *LEUART Callback to set
 ******************************************************************************/
void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len, uint32_t leuart_cb)
{
    while(leuart0_SM.busy);

    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();

    leuart0_SM.current_state = STRING_INIT;
    leuart0_SM.leuart = leuart;


    strcpy(leuart0_SM.data,string);

    leuart0_SM.data_sent = 0;
    leuart0_SM.str_length = string_len;
    leuart0_SM.leuart0_write_cb = leuart_cb;
    leuart0_SM.busy = true;
    sleep_block_mode(LEUART_TX_EM);

    leuart0_SM.leuart->IEN |= LEUART_IEN_TXBL;
    CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 * Reads whether SM Read is busy currently.
 * @details
 * Used for TDD and development for checking busy bit. Returns value of static LEUART_READ_SM struct.
 *
 *
 ******************************************************************************/
bool leuart_tx_busy()
{
  return leuart0_SM_READ.read_busy;
}

/***************************************************************************//**
 * @brief
 *IRQhandler for LEUART0 peripheral.
 * @details
 * Handles any interrupts triggered by the LEUART peripheral. Includes interrupts triggered for read and write operations, as well as completion of operations.
 *@note
 * TXC should only occur once transmission is fully complete. TXBL should occur once per character transmission. Read operations use STARFRAME,RXDATAV,SIGFRAME
 ******************************************************************************/
void LEUART0_IRQHandler(void)
{
  uint32_t interrupt_flag = LEUART0->IF & LEUART0->IEN;
  LEUART0->IFC = interrupt_flag;

  if(interrupt_flag & LEUART_IF_TXBL){
      TXBL_IRQ(&leuart0_SM);
    }
  if(interrupt_flag & LEUART_IF_TXC){
      TXC_IRQ(&leuart0_SM);
    }



  if(interrupt_flag & LEUART_IF_STARTF){
      STARTFRAME_HANDLER(&leuart0_SM_READ);
    }
  if(interrupt_flag & LEUART_IF_RXDATAV){
      RXDATAV_HANDLER(&leuart0_SM_READ);
    }

  if(interrupt_flag & LEUART_IF_SIGF){
      SIGFRAME_HANDLER(&leuart0_SM_READ);
    }
}

/***************************************************************************//**
 * @brief
 * Handler for TXBL interrupt for write operations.
 * @details
 * Handles TXBL interrupt sent during write operations. Checks to see if characters have been passed
 *then disables TXBL and clears TXC then enables to ensure proper operation.
 *
 * @param[in] LEUART_SM
 * Input state machine struct for LEUART_WRITE operation
 ******************************************************************************/
void TXBL_IRQ(LEUART_WRITE_SM *LEUART_SM){
  switch(LEUART_SM->current_state) {
    case STRING_INIT:
      LEUART_SM->current_state = write_op;
      break;
    case write_op:
      if(LEUART_SM->data_sent != LEUART_SM->str_length){
          leuart_app_transmit_byte(LEUART_SM->leuart, LEUART_SM->data[LEUART_SM->data_sent]);
          LEUART_SM->data_sent++;
      }
      else{
          LEUART0->IEN &= ~LEUART_IEN_TXBL;
          LEUART0->IFC |= LEUART_IEN_TXC;
          LEUART0->IEN |= LEUART_IEN_TXC;

          LEUART_SM->current_state = end;
      }
      break;
    default:
      EFM_ASSERT(false);
  }
}

/***************************************************************************//**
 * @brief
 *Handler for TXC interrupt for write operations.
 * @details
 * This function is only called by the IRQ handler when data transfer is complete, and will disable the TXC interrupt upon being called.
 * Additionally, event is added to scheduler at end of operation.
 * @param[in] LEUART_SM
 * Input state machine struct for LEUART_WRITE operation
 ******************************************************************************/
void TXC_IRQ(LEUART_WRITE_SM *LEUART_SM){
  switch(LEUART_SM->current_state) {
    case end:
      LEUART_SM->leuart->IEN &= ~(LEUART_IEN_TXC);

      sleep_unblock_mode(LEUART_TX_EM);
      LEUART_SM->busy = false;

      add_scheduled_event(LEUART_SM->leuart0_write_cb);
      break;
    default:
      EFM_ASSERT(false);
      break;
  }
}

/***************************************************************************//**
 * @brief
 * Handles STARTFRAME interrupts for read operations
 * @details
 * Blocks data reception, sets initial strlen, and data for RXDATA.
 * Enables SIGFRAME and RXDATAV interrupts.
 *
 * @param[in] LEUART_SM
 * Input state machine struct for LEUART_WRITE operation
 *
 ******************************************************************************/
static void STARTFRAME_HANDLER(LEUART_READ_SM *LEUART_SM){
  switch(LEUART_SM->current_read_state){
    case STARTFRAME:
      LEUART_SM->current_read_state = RXDATAV;

      LEUART_SM->leuart_read->IEN |= LEUART_IEN_RXDATAV;
      LEUART_SM->leuart_read->IEN |= LEUART_IEN_SIGF;

      LEUART_SM->leuart_read->CMD |= LEUART_CMD_RXBLOCKDIS;

      LEUART_SM->str_length = 0;
      LEUART_SM->read_str[LEUART_SM->str_length] = LEUART_SM->leuart_read->RXDATA;
      LEUART_SM->str_length++;
       break;
    default:
      EFM_ASSERT(false);
      break;
  }
}

/***************************************************************************//**
 * @brief
 * Handles SIGFRAME interrupts for read operations.
 * @details
 * Disables sigframe and rxdatav interrupts, re-enables rxblocken to prevent further reception of data.
 * Resets string length and returns state to startframe for additional operation. Adds scheduled event for finished data transmission
 *
 * @param[in] LEUART_SM
 * Input state machine struct for LEUART_WRITE operation
 *
 ******************************************************************************/
static void SIGFRAME_HANDLER(LEUART_READ_SM *LEUART_SM){
  switch(LEUART_SM->current_read_state){
    case RXDATAV:
      LEUART_SM->current_read_state = SIGFRAME;


      LEUART_SM->leuart_read->IEN &= ~LEUART_IEN_RXDATAV;
      LEUART_SM->leuart_read->IEN &= ~LEUART_IEN_SIGF;


      LEUART_SM->leuart_read->CMD |= LEUART_CMD_RXBLOCKEN;
      while(LEUART_SM->leuart_read->SYNCBUSY);
      LEUART_SM->read_str[LEUART_SM->str_length] = 0;
      LEUART_SM->str_length++;


      LEUART_SM->current_read_state = STARTFRAME;
      add_scheduled_event(BLE_TX_DONE_CB);
      break;
    default:
      EFM_ASSERT(false);
      break;
  }
}

/***************************************************************************//**
 * @brief
 *Handles RXDATAV interrupts for read operations that occur.
 * @details
 *Reads actual data for read operation, incrementing per operation.
 *
 * @param[in] LEUART_SM
 * Input state machine struct for LEUART_WRITE operation
 *
 ******************************************************************************/
static void RXDATAV_HANDLER(LEUART_READ_SM *LEUART_SM)
{
  switch(LEUART_SM->current_read_state)
  {
    case RXDATAV:
      LEUART_SM->read_str[LEUART_SM->str_length] = LEUART_SM->leuart_read->RXDATA;
      LEUART_SM->str_length++;
      break;
    default:
      EFM_ASSERT(false);
      break;
  }
}

/***************************************************************************//**
 * @brief
 *Returns data from private state machine struct in leuart.c
 * @details
 * Used for app.C callback function for RXdata. Allows data to be read outside of private file
 *
 * @param[in] *out_str
 * Input string to return data to, being read from private location.
 *
 ******************************************************************************/
void received_str(char * out_str)
{
  strcpy(out_str, leuart0_SM_READ.read_str);
}


/***************************************************************************//**
 * @brief
 *   LEUART STATUS function returns the STATUS of the peripheral for the
 *   TDD test
 *
 * @details
 * 	 This function enables the LEUART STATUS register to be provided to
 * 	 a function outside this .c module.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the STATUS register value as an uint32_t value
 *
 ******************************************************************************/
uint32_t leuart_status(LEUART_TypeDef *leuart){
	uint32_t	status_reg;
	status_reg = leuart->STATUS;
	return status_reg;
}

/***************************************************************************//**
 * @brief
 *   LEUART CMD Write sends a command to the CMD register
 *
 * @details
 * 	 This function is used by the TDD test function to program the LEUART
 * 	 for the TDD tests.
 *
 * @note
 *   Before exiting this function to update  the CMD register, it must
 *   perform a SYNCBUSY while loop to ensure that the CMD has by synchronized
 *   to the lower frequency LEUART domain.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] cmd_update
 * 	 The value to write into the CMD register
 *
 ******************************************************************************/
void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update){

	leuart->CMD = cmd_update;
	while(leuart->SYNCBUSY);
}

/***************************************************************************//**
 * @brief
 *   LEUART IF Reset resets all interrupt flag bits that can be cleared
 *   through the Interrupt Flag Clear register
 *
 * @details
 * 	 This function is used by the TDD test program to clear interrupts before
 * 	 the TDD tests and to reset the LEUART interrupts before the TDD
 * 	 exits
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 ******************************************************************************/
void leuart_if_reset(LEUART_TypeDef *leuart){
	leuart->IFC = 0xffffffff;
}

/***************************************************************************//**
 * @brief
 *   LEUART App Transmit Byte transmits a byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a transmit byte, a while statement checking for the TXBL
 *   bit in the Interrupt Flag register is required before writing the
 *   TXDATA register.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] data_out
 *   Byte to be transmitted by the LEUART peripheral
 *
 ******************************************************************************/
void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out){
	while (!(leuart->IF & LEUART_IF_TXBL));
	leuart->TXDATA = data_out;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Receive Byte polls a receive byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a receive byte, a while statement checking for the RXDATAV
 *   bit in the Interrupt Flag register is required before reading the
 *   RXDATA register.
 *
 * @param[in] leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the byte read from the LEUART peripheral
 *
 ******************************************************************************/
uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart){
	uint8_t leuart_data;
	while (!(leuart->IF & LEUART_IF_RXDATAV));
	leuart_data = leuart->RXDATA;
	return leuart_data;
}

/***************************************************************************//**
 * @brief
 *TDD function for LEUART read operation development
 *
 * @details
 * Tests LEUART READ STATE machine for reads and proper operation. Tests # and !
 * as startframe and sigframe characters, determines if correct interrupts are being raised
 * for read operations..
 *
 ******************************************************************************/
void leuart_rx_tdd()
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  LEUART0->CTRL |= LEUART_CTRL_LOOPBK;

  while(LEUART0->SYNCBUSY);
  uint32_t  startframe = LEUART0->STARTFRAME;
  uint32_t  sigframe = LEUART0 -> SIGFRAME;

  LEUART0->TXDATA = ~startframe;
  timer_delay(Tdelay);

  EFM_ASSERT(!(LEUART0->IF & LEUART_IF_RXDATAV));
  LEUART0->TXDATA = startframe;
  timer_delay(Tdelay);

  EFM_ASSERT(LEUART0->IF & LEUART_IF_RXDATAV);
  EFM_ASSERT(startframe == LEUART0->RXDATA);
  LEUART0->TXDATA = sigframe;
  timer_delay(Tdelay);
  EFM_ASSERT(LEUART0->IF & LEUART_IF_SIGF);
  EFM_ASSERT(sigframe == LEUART0->RXDATA);

  LEUART0->CMD |= LEUART_CMD_RXBLOCKEN;
  LEUART0->IFC |= LEUART_IF_STARTF | LEUART_IF_SIGF;
  while(LEUART0->SYNCBUSY);

  CORE_EXIT_CRITICAL();

  char test_str[] = "hello";

  char tx_str [20], expected_result[20];
  tx_str[0] = 0;

  strcat(tx_str, "abc");

  uint32_t str_length = strlen(tx_str);
  tx_str[str_length] = LEUART0->STARTFRAME;
  tx_str[str_length+1] = 0;
  strcat(tx_str,test_str);
  str_length = strlen(tx_str);
  tx_str[str_length] = LEUART0->SIGFRAME;
  tx_str[str_length+1] = 0;
  strcat(tx_str, "def");
  expected_result[0] = LEUART0->STARTFRAME;
  expected_result[1] = 0;
  strcat(expected_result, test_str);
  str_length = strlen(expected_result);


  expected_result[str_length] = LEUART0->SIGFRAME;
  expected_result[str_length+1] = 0;
  leuart_start(LEUART0, tx_str, strlen(tx_str), NULL_CB);

  while(leuart_tx_busy());
  timer_delay(TdelayLong);

  char final_str[20];
  strcpy(final_str, leuart0_SM_READ.read_str);

  EFM_ASSERT(!strcmp(final_str, expected_result));
  EFM_ASSERT(LEUART0->STATUS & LEUART_STATUS_RXENS);

  LEUART0->CTRL &= ~LEUART_CTRL_LOOPBK;
}
