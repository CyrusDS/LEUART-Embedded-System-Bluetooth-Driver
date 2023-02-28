/**
 * @file scheduler.c
 * @author Cyrus Sowdaey
 * @date 10/17/2021
 * @brief scheduler
 * Responsible for handling of event scheduling, removal, configuration.
 */



#include "scheduler.h"


//*******************
//private variables
//*******************

static unsigned int event_scheduled;
/***************************************************************************//**
 * @brief
 * scheduler_open() is used to set initial event_scheduled state.
 *
 * @details
 * event_scheduled is set to 0
 *
 * @note
 *Atomic operations occur so we need to run CORE ENTER and EXIT functions to ensure proper operation.
 *
 ******************************************************************************/
void scheduler_open(void) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled = 0;
  CORE_EXIT_CRITICAL();
  return;
}
/***************************************************************************//**
 * @brief
 *add_scheduled_event() takes an input event and determines new state of private variable event_scheduled
 *
 * @details
 *Performs a bitwise OR with the new event and existing eavent.
 *
 * @note
 *Atomic operations occur so we need to run CORE ENTER and EXIT functions to ensure proper operation.
 *
 * @param[in] event
 * event is a uint32_t type input. This is OR'd with event_scheduled
 ******************************************************************************/
void add_scheduled_event(uint32_t event) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled |= event;
  CORE_EXIT_CRITICAL();
  return;
}
/***************************************************************************//**
 * @brief
 *Removes a currently scheduled event from event_scheduled private variable.
 *
 * @details
 *event_scheduled ANDed with input event. Will remove an event as a result.
 *
 * @note
 *Atomic operations occur so we need to run CORE ENTER and EXIT functions to ensure proper operation.
 *
 * @param[in] event
 *event is a uint32_t type input. This is OR'd with event_scheduled
 ******************************************************************************/
void remove_scheduled_event(uint32_t event) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled = event_scheduled & ~event;
  CORE_EXIT_CRITICAL();
  return;
}
/***************************************************************************//**
 * @brief
 *Gets current state of event_scheduled static variable.
 *
 * @details
 *Returned as an output to the function in the type of uint32_t
 *
 * @note
 *Simply returns private event_scheduled. We are only reading the variable, does not require additional practices.
 *
 * @param[out] event_scheduled
 *This is a static variable which is declared in scheduler.c
 ******************************************************************************/
uint32_t get_scheduled_events(void) {
  return event_scheduled; //return state of private variable
}
