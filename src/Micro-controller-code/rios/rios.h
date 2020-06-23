/* FortunaOS: Scheduler

   Based on template for preemptive version of RIOS implemented on an
   AVR available from: http://www.cs.ucr.edu/~vahid/rios/rios_avr.htm

   For Copyright and License see end of rios.c file. 


   Usage:
     - Tasks can be added before or after scheduler has been initialized.
     - Global Iterrupts need to be enabled manually. 
     - Tasks can be added while the scheduler is running.

*/

#ifndef RIOS_H
#define RIOS_H

/* Limit for number of tasks: int8_t */
#define MAX_TASKS 10


/* Comment out the following line to get more precise 1 ms periods,
   but loose brightness adjustement of LED 
*/
#define OS_LED_BRIGHTNESS

void os_init_scheduler();

void os_led_brightness(uint8_t brightness);


/* Returns task priority (lower is higher) or -1 if not successful: */
int os_add_task(int (*fnc)(int), uint32_t period_ms, int startState);


#endif /* RIOS_H */
