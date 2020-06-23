/* FortunaOS: Scheduler

   Template for preemptive version of RIOS implemented on an AVR
   available from: http://www.cs.ucr.edu/~vahid/rios/rios_avr.htm

   Slightly modified by Klaus-Peter Zauner, Feb 2014, Mar 2015.

   For Copyright and License see end of file. 

*/

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <util/delay.h>
#include "rios.h"




typedef struct task {
   uint8_t running;      /* 1 indicates task is running */
   uint32_t period_ms;   /* Rate at which the task should tick in ms*/
   uint32_t elapsedTime; /* Time since task's previous tick */
   int (*TaskFct)(int);  /* Function to call for task's tick */
   int state;            /* Current state of state machine */
} task;

task tasks[MAX_TASKS];
int8_t tasksNum = -1;



uint8_t runningTasks[MAX_TASKS+1] = {255}; /* Track running tasks, [0] always idleTask */
const uint32_t idleTask = 255;             /* 0 highest priority, 255 lowest */
uint8_t currentTask = 0;                   /* Index of highest priority task in runningTasks */

unsigned schedule_time = 0;
ISR(TIMER0_OVF_vect) {
   uint8_t i;

   for (i=0; i <= tasksNum; ++i) { /* Heart of scheduler code */
      if (  (tasks[i].elapsedTime >= tasks[i].period_ms) /* Task ready */
          && (runningTasks[currentTask] > i) /* Task priority > current task priority */
          && (!tasks[i].running)             /* Task not already running (no self-preemption) */
         ) {
   
         cli();
         tasks[i].elapsedTime = 0;      /* Reset time since last tick */
         tasks[i].running = 1;          /* Mark as running */
         currentTask += 1;
         runningTasks[currentTask] = i; /* Add to runningTasks */
         sei();
   
         tasks[i].state = tasks[i].TaskFct(tasks[i].state); /* Execute tick */
            
         cli();
         tasks[i].running = 0;                 /* Mark as not running */
         runningTasks[currentTask] = idleTask; /* Remove from runningTasks */
         currentTask -= 1;
         sei();
   
      }
      tasks[i].elapsedTime += 1;
   }

   
}



#ifdef OS_LED_BRIGHTNESS

/* Use PWM mode to also control LED brightness from Timer 0 */
void os_init_scheduler() {

    /* Configure 8 bit Timer 0 for ISR  */
	TCCR0A = _BV(COM0A1)    /* Clear OCA0A on Compare match, set on TOP */
           | _BV(WGM01)		/* fast PWM mode */
           | _BV(WGM00);

    TCCR0B |= _BV(CS00)     
            | _BV(CS01);   /* F_CPU/64, DS p.112 */


    /* Interrupts at 488.3 Hz: FCPU/(510*N) with N=64, DS p.105 */  

    TIMSK0 = _BV(TOIE0); /* enable overflow interrupt for T0, DS p.113  */
    TCNT0 = 0;
    OCR0A = 255;  /* LED full brightness */
}

void os_led_brightness(uint8_t level) {
	if (level) {
		OCR0A = level;
		DDRB  |=  _BV(PINB7); 
	} else {
		DDRB  &=  ~_BV(PINB7); 
	}
}


#else


/* Use CTC mode to have periods of exactly 1 ms */
void os_init_scheduler() {

    /* Configure 8 bit Timer 0 for 1 ms ISR  */
	TCCR0A |= _BV(WGM01);   /* Clear Timer on Compare match (CTC, Mode 2), DS p.111 */
    TCCR0B |= _BV(CS00)     
            | _BV(CS01);   /* F_CPU/64, DS p.112 */

    OCR0A = (uint8_t)(F_CPU / (64UL * 1000) - 1); /* 1 kHz interrupts */

    TIMSK0 = _BV(OCIE0A); /* enable compare match interrupt for T0, DS p.113  */
    TCNT0 = 0;

}


void os_led_brightness(uint8_t brightness) {}

#endif	/* OS_LED_BRIGHTNESS */



int os_add_task(int (*fnc)(int), uint32_t period_ms, int initState) {
   int t;

   t = tasksNum + 1;
 
   if (t >= MAX_TASKS) {
	   t = -1;
   } else {

   #ifdef  OS_LED_BRIGHTNESS
	  tasks[t].period_ms = 488.3/1000.0 * period_ms + 0.5;
   #else
	  tasks[t].period_ms = period_ms;
   #endif	/* OS_LED_BRIGHTNESS */

	  tasks[t].elapsedTime = tasks[t].period_ms;
	  tasks[t].running = 0;
	  tasks[t].TaskFct = fnc;
	  tasks[t].state = initState;
	  tasksNum = t; /* New task fully initialized */
   }
   
   return t;
}


/*
   Copyright (c) 2013 Frank Vahid, Tony Givargis, and
   Bailey Miller. Univ. of California, Riverside and Irvine.
   RIOS version 1.2

   Copyright (c) 2012 UC Regents. All rights reserved.

      Developed by: Frank Vahid, Bailey Miller, and Tony Givargis
      University of California, Riverside; University of California, Irvine
      <http://www.riosscheduler.org>http://www.riosscheduler.org

      Permission is hereby granted, free of charge, to any person
      obtaining a copy of this software and associated documentation
      files (the "Software"), to deal with the Software without
      restriction, including without limitation the rights to use,
      copy, modify, merge, publish, distribute, sublicense, and/or sell
      copies of the Software, and to permit persons to whom the
      Software is furnished to do so, subject to the following conditions:

      * Redistributions of source code must retain the above
        copyright notice, this list of conditions and the following
        disclaimers.
      * Redistributions in binary form must reproduce the above
        copyright notice, this list of conditions and the following
        disclaimers in the documentation and/or other materials
        provided with the distribution.
      * Neither the names of any of the developers or universities nor
        the names of its contributors may be used to endorse or
        promote products derived from this Software without
        specific prior written permission.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
      EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
      OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
      NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT
      HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
      WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
      FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
      OTHER DEALINGS WITH THE SOFTWARE.
   (http://opensource.org/licenses/NCSA)

*/