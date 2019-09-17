#pragma once
#ifndef THOR_EXTI_HPP
#define THOR_EXTI_HPP

#if defined( THOR_STM32HAL_DRIVERS ) && ( THOR_STM32HAL_DRIVERS == 1 )

/* Boost Includes */
#include <boost/circular_buffer.hpp>
#include <boost/container/vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

/* Thor Includes */
#include <Thor/config.hpp>
#include <Thor/definitions/interrupt_definitions.hpp>


#if defined( USING_FREERTOS )

#ifdef __cplusplus
extern "C"
{
#endif

#include "FreeRTOS.h"
#include "semphr.h"

#ifdef __cplusplus
}
#endif

using namespace Thor::Interrupt;

class TaskTrigger;
extern boost::circular_buffer<TaskTrigger *> TriggerBuffer;

/** Implements a convenient way to trigger a FreeRTOS task upon an event generated by another peripheral. */
class TaskTrigger
{
public:
  /** Informs TaskTrigger that an event has occured. Typically this is called from an ISR routine.
   *	@param[in] trig		The source trigger from which the event was generated, of type Thor::Interrupt::Trigger
   *	@param[in] objAddr	The address of a valid TaskTrigger object that holds the semaphores logged with attachEventConsumer
   **/
  void logEvent( Trigger trig, TaskTrigger *objAddr )
  {
    /* A semaphore has to be attached with attachEventConsumer before this will work */
    if ( semaphores[ trig ] )
    {
      pendingTask.push_back( trig );         // This logs which trigger generated the request
      TriggerBuffer.push_back( objAddr );    // This lets the ISR know what class object to use

      /** Trigger the EXTI0 Interrupt to signal data ready. Because it has a lower priority
       *	than whatever ISR is calling, it will not immediately fire. This assumes that the
       *	user does not manually set a peripheral ISR to be lower than EXTI0_MAX_IRQn_PRIORITY
       **/
      LL_EXTI_GenerateSWI_0_31( LL_EXTI_LINE_0 );
    }
  }

  /** Ataches a semaphore to an interrupt source. When the source is triggered, it will automatically
   *	'give' to that semaphore and thus release a waiting thread to execute. If that thread has a higher
   *	priority than EXTI0 handler (not likely), a context switch is requested. Currently only 1 semaphore
   *	per trigger is allowed.
   *
   *	@param[in] trig		The source trigger from which events will be generated, of type
   *Thor::Interrupt::Trigger
   *	@param[in] semphr	Address of the semaphore object to be 'given' to upon triggering
   **/
  void attachEventConsumer( Trigger trig, SemaphoreHandle_t *semphr )
  {
    semaphores[ trig ] = semphr;
  }

  /** Self-explanatory. It removes the trigger as an event consumer.
   *	@param[in] trig	The source trigger to remove, of type Thor::Interrupt::Trigger
   **/
  void removeEventConsumer( Trigger trig )
  {
    semaphores[ trig ] = nullptr;
  }

  /** An ISR only function that returns the next semaphore to be triggered
   *	@return semaphore address
   **/
  SemaphoreHandle_t *getEventSemaphore()
  {
    SemaphoreHandle_t *tempSem = nullptr;

    if ( !pendingTask.empty() )
    {
      tempSem = semaphores[ pendingTask.front() ];
      pendingTask.pop_front();
    }

    return tempSem;
  }

  TaskTrigger()
  {
    pendingTask.set_capacity( MAX_PENDING_TASK_TRIGGERS );
  }

  ~TaskTrigger() = default;

private:
  boost::circular_buffer<Trigger> pendingTask;
  SemaphoreHandle_t *semaphores[ Trigger::MAX_SOURCES ] = { nullptr };
};


extern void setupEXTI0_Interrupt();

#endif /* USING_FREERTOS */


#ifdef __cplusplus
extern "C"
{
#endif
  extern void EXTI0_IRQHandler();
  extern void EXTI1_IRQHandler();
  extern void EXTI2_IRQHandler();
  extern void EXTI3_IRQHandler();
  extern void EXTI4_IRQHandler();
  extern void EXTI9_5_IRQHandler();
  extern void EXTI15_10_IRQHandler();
#ifdef __cplusplus
}
#endif

#endif 

#endif /* THOR_EXTI_HPP */
