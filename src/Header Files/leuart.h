//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef	LEUART_GUARD_H
#define	LEUART_GUARD_H

#include "em_leuart.h"
#include "sleep_routines.h"
#include "ble.h"
#include "HW_delay.h"
#include "app.h"


//***********************************************************************************
// defined files
//***********************************************************************************

#define LEUART_TX_EM		EM3
#define LEUART_RX_EM		EM3
#define Tdelay    2
#define TdelayLong    50
#define IFC_CLR       0xFF

/***************************************************************************//**
 * @addtogroup leuart
 * @{
 ******************************************************************************/

typedef struct {
  uint32_t          refFreq;
	uint32_t					baudrate;
	LEUART_Databits_TypeDef		databits;
	LEUART_Enable_TypeDef		enable;
	LEUART_Parity_TypeDef 		parity;
	LEUART_Stopbits_TypeDef		stopbits;
	bool						rxblocken;
	bool						sfubrx;
	bool						startframe_en;
	char						startframe;
	bool						sigframe_en;
	char						sigframe;
	uint32_t					rx_loc;
	uint32_t					rx_pin_en;
	uint32_t					tx_loc;
	uint32_t					tx_pin_en;
	bool						  rx_en;
	bool				   		tx_en;
	uint32_t					rx_done_evt;
	uint32_t					tx_done_evt;
} LEUART_OPEN_STRUCT;

typedef enum {
  STRING_INIT,
  write_op,
  end,
} LEUART_WRITE_STATES;

typedef enum {
  STARTFRAME,
  RXDATAV,
  SIGFRAME
} LEUART_READ_STATES ;

typedef struct {
  LEUART_WRITE_STATES current_state;
  LEUART_TypeDef *leuart;

  char data[80];
  uint32_t str_length;
  uint32_t leuart0_write_cb;
  volatile bool busy;
  uint32_t data_sent;
} LEUART_WRITE_SM;

typedef struct {
  LEUART_READ_STATES current_read_state;
  LEUART_TypeDef *leuart_read;

  uint32_t leuart0_read_cb;
  volatile bool read_busy;
  char read_str[80];
//  uint32_t sigframe;
//  uint32_t startframe;
  uint32_t str_length;
} LEUART_READ_SM;

/** @} (end addtogroup leuart) */

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings);
void LEUART0_IRQHandler(void);
void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len, uint32_t leuart_cb);

bool leuart_tx_busy(void);


uint32_t leuart_status(LEUART_TypeDef *leuart);
void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update);
void leuart_if_reset(LEUART_TypeDef *leuart);
void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out);
uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart);


void TXBL_IRQ(LEUART_WRITE_SM *LEUART_SM);
void TXC_IRQ(LEUART_WRITE_SM *LEUART_SM);

void leuart_rx_tdd(void);
void received_str(char *out_str);

#endif
