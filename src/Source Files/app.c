/**
 * @file app.c
 * @author Cyrus Sowdaey
 * @date 10/17/2021
 * @brief File responsible for functions to run all previously defined scheduling, energy mode, clock management, and gpio configuration functions.
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h" //encapsulation, all other needed files defined within here.


//***********************************************************************************
// defined files
//***********************************************************************************
//#define BLE_TEST_ENABLED
static uint32_t x=3;
static uint32_t y=0;

//***********************************************************************************
// Private variables
//***********************************************************************************
//static unsigned int color = 0; //declare unsigned int to represent the current color of 3 settings: 0,1,2.

//***********************************************************************************
// Private functions
//***********************************************************************************

static void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route); //declaration of defined function, shown later.

//***********************************************************************************
// Global functions
//***********************************************************************************

 /***************************************************************************//**
 * @brief
 *app_peripheral_setup() is used to run all the setup functions: the scheduling, energy mode, clock management, and gpio configuration functions.
 *
 * @details
 *Running functions which are changing registers and enabling the timer, specifically timer0. Additionally, configure scheduler to be open and set lowest_energy mode.
 *Configures GPIO and RGB pins through gpio_open and rgb_init.
 * @note
 * This functions purpose is purely to run other functions, as each configuration for specific peripherals and the timer is separate.
 *
 ******************************************************************************/
void app_peripheral_setup(void){
  scheduler_open();
  sleep_open();
  cmu_open();
  gpio_open();
  Si1133_i2c_open();

  rgb_init();
  sleep_block_mode(SYSTEM_BLOCK_EM);
  ble_open(TX_CALLBACK, RX_CALLBACK);
  app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);
  add_scheduled_event(BOOT_UP_CB);
}

/***************************************************************************//**
 * @brief
 *Function to set up routing to pre defined variables and numbers so that we have the correct info inside the struct.
 *
 * @details
 *Sets values to letimer_pwm_struct parameters as well as initialize the struct itself.
 *
 * @note
 *This function purely deals with the letimer_pwm_struct and configuring it. The end of the function enters the struct into letimer_pwm_open()
 *
 * @param[in] period
 *Total period is the PWM cycle defined in app.h
 *
 * @param[in] out0_route
 *Location used to route LETIMER0 outputs to the correct gecko output pins. Type is uint32_t
 * @param[in] out1_route
 *Location used to route LETIMER0 outputs to the correct gecko output pins. Type is uint32_t
 ******************************************************************************/

void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route){
  // Initializing LETIMER0 for PWM operation by creating the
  // letimer_pwm_struct and initializing all of its elements
  // APP_LETIMER_PWM_TypeDef is defined in letimer.h
  APP_LETIMER_PWM_TypeDef   letimer_pwm_struct; //initial struct declaration
  //letimer_pwm_struct is the struct name

  letimer_pwm_struct.debugRun = false;
  letimer_pwm_struct.enable = false; // disable until config finished

  letimer_pwm_struct.out_pin_0_en = true;
  letimer_pwm_struct.out_pin_1_en = true;

  letimer_pwm_struct.period = period;
  letimer_pwm_struct.active_period = act_period;

  letimer_pwm_struct.out_pin_route0 = out0_route;
  letimer_pwm_struct.out_pin_route1 = out1_route;

  letimer_pwm_struct.uf_irq_enable = true;
  letimer_pwm_struct.comp1_irq_enable = true;
  letimer_pwm_struct.comp0_irq_enable = false;

  letimer_pwm_struct.comp0_cb = LETIMER0_COMP0_CB;
  letimer_pwm_struct.comp1_cb = LETIMER0_COMP1_CB;
  letimer_pwm_struct.uf_cb = LETIMER0_UF_CB;


  letimer_pwm_open(LETIMER0, &letimer_pwm_struct);
}

/***************************************************************************//**
 * @brief
 *Used to turn the rgb LED on and cycle through the modes 0,1,2 then repeat after cycling.
 *
 * @details
 *Specific color order is R->G->B->R.. repeating.
 *
 * @note
 *Here, the UF interrupt is being used to cycle the blinking states.
 *
 ******************************************************************************/
void scheduled_letimer0_uf_cb(void){
  /*
  if (color == 0) {
      leds_enabled(RGB_LED_1, COLOR_RED, false);
      color++;
  }
  else if (color == 1) {
        leds_enabled(RGB_LED_1, COLOR_GREEN, false);
        color++;
    }
  else if (color == 2) {
          leds_enabled(RGB_LED_1, COLOR_BLUE, false);
          color = 0;
      }
      */
  SI1133_request_result(SI1133_CB);
  x = x+3;
  y = y+1;
  float z = (float) x/y;

  char data[80];
  sprintf(data, ", z = %.1f\n", z);
  ble_write(data);

}
/***************************************************************************//**
 * @brief
 *LETIMER COMP0 interrupt is not being used. This functions exists as an assert check.
 *
 * @details
 *EFM_Assert(False) will always trap us here if we ever enter it. We do not want to enter this.
 *
 * @note
 *Used as a diagnostic tool. Assert is contained within.
 *
 ******************************************************************************/
void scheduled_letimer0_comp0_cb(void) {
 // EFM_ASSERT(false);
}
/***************************************************************************//**
 * @brief
 *Turns on RGB_LED_1 and rotating through RGB colors when ran.
 *
 * @details
 *Uses leds_enabled function in order to correctly set enabled leds based on state of color variable.
 *
 * @note
 *Uses COMP1 Interrupt, enabled.
 *
 ******************************************************************************/
void scheduled_letimer0_comp1_cb(void) {
/*
  if (color == 0) {
       leds_enabled(RGB_LED_1, COLOR_RED, true);
   }

  else if (color == 1) {
       leds_enabled(RGB_LED_1, COLOR_GREEN, true);
     }
  else if (color == 2) {
       leds_enabled(RGB_LED_1, COLOR_BLUE, true);
     }
     */
  //Si1133_read(READ_DATA_B, PART_ID_REGISTER, SI1133_CB);
  Si1133_force();
}


/***************************************************************************//**
 * @brief
 * Callback function upon completion of an i2c read operation on the SI1133
 *
 * @details
 * Reads value from si1133 peripheral and displays green if expected value is read. Turns on RED led if not properly able to read.

 * @note
 * With successful operation, should always be green.
 ******************************************************************************/
void scheduled_si1133_read_cb(){
  uint32_t si1133_data = Si1133_read_result();
/*
  if(si1133_data == EXPECTED_DATA){
      leds_enabled(RGB_LED_1, COLOR_GREEN, true);
  }else{
      leds_enabled(RGB_LED_1, COLOR_RED, true);
  }

  if(si1133_data < EXPECTED_DATA){
      leds_enabled(RGB_LED_1, COLOR_BLUE, true);
  }else{
      leds_enabled(RGB_LED_1, COLOR_BLUE, false);
  }
*/

  if(si1133_data < EXPECTED_DATA){

      leds_enabled(RGB_LED_1, COLOR_BLUE, true);
      char data[80];

      int int_data = si1133_data;

      sprintf(data, "It's Dark outside = %d", int_data);
      ble_write(data);
  }else{
      leds_enabled(RGB_LED_1, COLOR_BLUE, false);
      char data[80];

      int int_data = si1133_data;

      sprintf(data, "It's Light outside = %d", int_data);
      ble_write(data);
  }



}

/***************************************************************************//**
 * @brief
 * Callback function for booting up peripheral with correct name scheme, a test string, and enabling a timer.
 *
 * @details
 * Performs a write of "Hello World" as a test to the BLE peripheral, as well as starting the letimer0.
 * @note
 * With successful operation, "Hello World" will be printed, and the BLE peripheral name should also be changed if it is not commented out.
 ******************************************************************************/
void scheduled_boot_up_cb(void) {
#ifdef BLE_TEST_ENABLED
  char ble_mod_name[13] = "CSUARTSENS";
  bool ble_result = ble_test(ble_mod_name);
  EFM_ASSERT(ble_result);
  timer_delay(DELAYTIME);
#endif
  ble_write("\n Hello World \n");
  letimer_start(LETIMER0, true);
}

/***************************************************************************//**
 * @brief
 * Handles BLE done callback
 *
 * @details
 * Checks assertion to determine if callback and scheduled events are nonzero
 *
 ******************************************************************************/
void BLE_TX_cb(void)
{
  EFM_ASSERT(!(get_scheduled_events() & BLE_TX_DONE_CB));
}



/***************************************************************************//**
 * @brief
 *BLE RX CB handling, taking incoming ASCII data.
 *
 * @details
 * Checks third value of string to either + or -, slows or speeds up based on this value.
 * Important for specific command we wish to issue, speeding up or slowing down by a specific amount specified
 * after the + or -. Calls letimer function to change period value as a result.
 ******************************************************************************/
void BLE_RX_cb(void){
  char private_input[80];
  int added_pwm = 0;

  received_str(private_input);

  if(private_input[1] == 'U'){
     if(private_input[2] == '+'){
         added_pwm = ((private_input[3] - 0x30)*100);
         added_pwm = added_pwm +((private_input[4] - 0x30)*10)+(private_input[5] - 0x30);
     }
     if(private_input[2] == '-'){
         added_pwm  = (((private_input[3] - 0x30)*100)+((private_input[4] - 0x30)*10)+(private_input[5] - 0x30));
         added_pwm  = added_pwm * (-1);
     }
  }
  letimer0_period(LETIMER0, added_pwm);
}


