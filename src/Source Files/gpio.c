/**
 * @file gpio.c
 * @author Cyrus Sowdaey
 * @date 10/17/2021
 * @brief GPIO Setup file
 *Responsible for pinModes being set for the LED pins GPIO
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************


//***********************************************************************************
// functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *Enabling GPIO and pin assignments for the red and green led.
 *
 * @details
 *Enables clock GPIO and configured LED pins so that we are able to send a PWM signal to them using the LETIMER0.
 *
 * @note
 *Pin assignments are done through enumeration references.
 *
 ******************************************************************************/

void gpio_open(void){

  CMU_ClockEnable(cmuClock_GPIO, true);

  //Configure  TX UART
  GPIO_DriveStrengthSet(LEUART_TX_PORT, LEUART_TX_DRIVE_STRENGTH);
  GPIO_PinModeSet(LEUART_TX_PORT, LEUART_TX_PIN, LEUART_TX_GPIOMODE, LEUART_TX_DEFAULT);

  GPIO_PinModeSet(LEUART_RX_PORT, LEUART_RX_PIN, LEUART_RX_GPIOMODE, LEUART_RX_DEFAULT);



  // Configure LED pins
  GPIO_DriveStrengthSet(LED_RED_PORT, LED_RED_DRIVE_STRENGTH);
  GPIO_PinModeSet(LED_RED_PORT, LED_RED_PIN, LED_RED_GPIOMODE, LED_RED_DEFAULT);

  GPIO_DriveStrengthSet(LED_GREEN_PORT, LED_GREEN_DRIVE_STRENGTH);
  GPIO_PinModeSet(LED_GREEN_PORT, LED_GREEN_PIN, LED_GREEN_GPIOMODE, LED_GREEN_DEFAULT);

  //Configure SI1133 sensor pin modes
  GPIO_DriveStrengthSet(SI1133_SENSOR_EN_PORT, gpioDriveStrengthWeakAlternateWeak);

  // Set RGB LED gpiopin configurations
  GPIO_PinModeSet(RGB_ENABLE_PORT, RGB_ENABLE_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
  GPIO_PinModeSet(RGB0_PORT, RGB0_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
  GPIO_PinModeSet(RGB1_PORT, RGB1_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
  GPIO_PinModeSet(RGB2_PORT, RGB2_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
  GPIO_PinModeSet(RGB3_PORT, RGB3_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
  GPIO_PinModeSet(RGB_RED_PORT, RGB_RED_PIN, gpioModePushPull, COLOR_DEFAULT_OFF);
  GPIO_PinModeSet(RGB_GREEN_PORT, RGB_GREEN_PIN, gpioModePushPull, COLOR_DEFAULT_OFF);
  GPIO_PinModeSet(RGB_BLUE_PORT, RGB_BLUE_PIN, gpioModePushPull, COLOR_DEFAULT_OFF);
//SDL SCA for I2C
  GPIO_PinModeSet(SI1133_SENSOR_EN_PORT,SI1133_SENSOR_EN_PIN, gpioModePushPull, gpioDriveStrengthWeakAlternateWeak); //configure pin modes for ambient light sensor
  GPIO_PinModeSet(SI1133_SCL_PORT,SI1133_SCL_PIN, gpioModeWiredAnd, SI1133_SCL_DEFAULT_EN); //configure pin mode for SCL and use wiredAnd to initiate conversation
  GPIO_PinModeSet(SI1133_SDA_PORT,SI1133_SDA_PIN, gpioModeWiredAnd, SI1133_SDA_DEFAULT_EN);//configure pin mode for SDA and use wiredAnd to initiate conversation
}
