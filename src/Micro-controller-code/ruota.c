/* ruota.c

    Copyright Peter Dannegger (danni@specs.de)
    http://www.mikrocontroller.net/articles/Entprellung 
    http://www.mikrocontroller.net/articles/Drehgeber

    Slightly adapted by Klaus-Peter Zauner for FortunaOS, March 2015

*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "rios.h"
#include "ruota.h"


#define ROTA	PE4
#define ROTB	PE5

#define COMPASS_SWITCHES (_BV(SWW)|_BV(SWS)|_BV(SWE)|_BV(SWN))
#define ALL_SWITCHES (_BV(SWC) | COMPASS_SWITCHES | _BV(OS_CD))


#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 


int8_t os_enc_delta(void);
volatile int8_t delta;


volatile uint8_t switch_state;   /* debounced and inverted key state:
                                 bit = 1: key pressed */
volatile uint8_t switch_press;   /* key press detect */
volatile uint8_t switch_rpt;     /* key long press and repeat */


volatile uint8_t extern_switch_state_D;
volatile uint8_t extern_switch_state_F;
 


int scan_encoder(int state);
int scan_switches(int state);
int print_switches(int state);
void init_external_switches(void);

uint8_t get_extern_switch_D(void)
{
   return extern_switch_state_D;
}

uint8_t get_extern_switch_F(void)
{
   return extern_switch_state_F;
}

void os_init_ruota(void) {

    /* Configure I/O Ports */
	DDRE &= ~_BV(ROTA) & ~_BV(ROTB);  /* Rot. Encoder inputs */
	PORTE |= _BV(ROTA) | _BV(ROTB);   /* Rot. Encoder pull-ups */

	DDRE &= ~_BV(SWC);   /* Central button */
	PORTE |= _BV(SWC);
	
	DDRC &= ~COMPASS_SWITCHES;  /* configure compass buttons for input */
	PORTC |= COMPASS_SWITCHES;  /* and turn on pull up resistors */
 
	DDRB &= ~_BV(OS_CD);   /* SD Card detection */
	PORTB |= _BV(OS_CD);


   init_external_switches();




	/* Schedule encoder scan evry 2 ms */
	os_add_task( scan_encoder,  2, 0);
	/* Schedule button scan at 10 ms */
	os_add_task( scan_switches, 10, 0);

}

void init_external_switches(void)
{
   /* Init external switches */
   /* Port D */
   DDRD &= ~(_BV(DDD6) | _BV(DDD4) | _BV(DDD3) | _BV(DDD2) | _BV(DDD1) | _BV(DDD0));
   PORTD |= _BV(PORTD6) |_BV(PORTD4) | _BV(PORTD3) | _BV(PORTD2) | _BV(PORTD1) | _BV(PORTD0);

   /* Port F */
   DDRF &= ~(_BV(DDF3) | _BV(DDF2) | _BV(DDF1) | _BV(DDF0));
   PORTF |= _BV(PORTF3) | _BV(PORTF2) | _BV(PORTF1) | _BV(PORTF0);
}

int print_switches(int state)
{

   if (extern_switch_state_D & _BV(PIND0))
      printByte("BTN 1 ON: ", PIND, 10);
   else
   {
      printByte("BTN 1: OFF", PIND, 10);   
   }

   if (extern_switch_state_D & _BV(PIND1))
      printByte("BTN 3 ON: ", PIND, 20);
   else
   {
      printByte("BTN 3 OFF: ", PIND, 20);   
   }

      if (extern_switch_state_D & _BV(PIND2))
      printByte("BTN 5 ON: ", PIND, 30);
   else
   {
      printByte("BTN 5 OFF: ", PIND, 30);   
   }

   if (extern_switch_state_D & _BV(PIND3))
      printByte("BTN 7 ON: ", PIND, 40);
   else
   {
      printByte("BTN 7 OFF: ", PIND, 40);   
   }

      if (extern_switch_state_D & _BV(PIND4))
      printByte("BTN 9 ON: ", PIND, 50);
   else
   {
      printByte("BTN 9 OFF: ", PIND, 50);   
   }

      if (extern_switch_state_F & _BV(PINF0))
      printByte("BTN 2 ON: ", PINF, 60);
   else
   {
      printByte("BTN 2 OFF: ", PINF, 60);   
   }

   if (extern_switch_state_F & _BV(PINF1))
      printByte("BTN 4 ON: ", PINF, 70);
   else
   {
      printByte("BTN 4 OFF: ", PINF, 70);   
   }

      if (extern_switch_state_F & _BV(PINF2))
      printByte("BTN 6 ON: ", PINF, 80);
   else
   {
      printByte("BTN 6 OFF: ", PINF, 80);   
   }

   if (extern_switch_state_F & _BV(PINF3))
      printByte("BTN 8 ON: ", PINF, 90);
   else
   {
      printByte("BTN 8 OFF: ", PINF, 90);   
   }

   if (extern_switch_state_D & _BV(PIND6))
      printByte("BTN 10 ON: ", PIND, 100);
   else
   {
      printByte("BTN 10 OFF: ", PIND, 100);   
   }



}


int scan_encoder(int state) {
     static int8_t last;
     int8_t new, diff;
     uint8_t wheel;

     cli();
     wheel = PINE;
     new = 0;
     if( wheel  & _BV(ROTB) ) new = 3;
     if( wheel  & _BV(ROTA) )
	 new ^= 1;		        	/* convert gray to binary */
     diff = last - new;			/* difference last - new */
     if( diff & 1 ){			/* bit 0 = value (1) */
	     last = new;		       	/* store new as next last */
	     delta += (diff & 2) - 1;	/* bit 1 = direction (+/-) */
     }
     sei();
     
     return state;
}


/* Read the two step encoder
   -> call frequently enough to avoid overflow of delta
*/
int8_t os_enc_delta() {
    int8_t val;

    cli();
    val = delta;
    delta &= 1;
    sei();

    return val >> 1;
}

void printByte(char* leading, uint8_t byte, uint8_t y){
      char printstr[100];
      sprintf(printstr, "%s: "BYTE_TO_BINARY_PATTERN, leading, BYTE_TO_BINARY(byte));
      display_string_xy(printstr, 50, y);
}



int scan_switches(int state) {
  static uint8_t ct0, ct1, rpt;
  uint8_t i;
 
  cli();


  /* 
     Overlay port E for central button of switch wheel and Port B
     for SD card detection switch:
  */ 
  i = switch_state ^ ~( (PINC|_BV(SWC)|_BV(OS_CD))	\
                   & (PINE|~_BV(SWC)) \
                   & (PINB|~_BV(OS_CD)));  /* switch has changed */
 // printByte("i: ", i, 20);
  ct0 = ~( ct0 & i ); 
  /* reset or count ct0 */
  ct1 = ct0 ^ (ct1 & i);                   /* reset or count ct1 */
 // printByte("ct1: ", ct1, 60);   
  i &= ct0 & ct1;                          /* count until roll over ? */
//  printByte("i: ", i, 80);   
  switch_state ^= i;                       /* then toggle debounced state */
  //  printByte("switch state: ", switch_state, 100);   
  switch_press |= switch_state & i;        /* 0->1: key press detect */
 
  if( (switch_state & ALL_SWITCHES) == 0 )     /* check repeat function */
     rpt = REPEAT_START;                 /* start delay */
  if( --rpt == 0 ){
    rpt = REPEAT_NEXT;                   /* repeat delay */
    switch_rpt |= switch_state & ALL_SWITCHES;
  }



   /* EXTERNAL SWITCHES PORTD */

   static uint8_t ct2, ct3;
   uint8_t state_changed_D;
   
   state_changed_D = extern_switch_state_D ^ ~(PIND);
   ct2 = ~(ct2 & state_changed_D);
   ct3 = ct2 ^ (ct3 & state_changed_D);
   state_changed_D &= ct2 & ct3;
   extern_switch_state_D ^= state_changed_D;
   


   /* EXTERNAL SWITCHES PORTF */

   static uint8_t ct4, ct5;
   uint8_t state_changed_F;
   
   state_changed_F = extern_switch_state_F ^ ~(PINF);
   ct4 = ~(ct4 & state_changed_F);
   ct5 = ct4 ^ (ct5 & state_changed_F);
   state_changed_F &= ct4 & ct5;
   extern_switch_state_F ^= state_changed_F;
   
  sei();
  
  return state;
}




/*
   Check if a key has been pressed
   Each pressed key is reported only once.
*/
uint8_t get_switch_press( uint8_t switch_mask ) {
  cli();                         /* read and clear atomic! */
  switch_mask &= switch_press;         /* read key(s) */
  switch_press ^= switch_mask;         /* clear key(s) */
  sei();
  return switch_mask;
}




/*
   Check if a key has been pressed long enough such that the
   key repeat functionality kicks in. After a small setup delay
   the key is reported being pressed in subsequent calls
   to this function. This simulates the user repeatedly
   pressing and releasing the key.
*/
uint8_t get_switch_rpt( uint8_t switch_mask ) {
  cli();                       /* read and clear atomic! */
  switch_mask &= switch_rpt;         /* read key(s) */
  switch_rpt ^= switch_mask;         /* clear key(s) */
  sei();
  return switch_mask;
}

 
/*
   Check if a key is pressed right now
*/
uint8_t get_switch_state( uint8_t switch_mask ) {
	switch_mask &= switch_state;
	return switch_mask;
}

 
/*
   Read key state and key press atomic!
*/
uint8_t get_switch_short( uint8_t switch_mask ) {
  cli();                                         
  return get_switch_press( ~switch_state & switch_mask );
}

 
/*
    Key pressed and held long enough that a repeat would
    trigger if enabled. 
*/
uint8_t get_switch_long( uint8_t switch_mask ) {
  return get_switch_press( get_switch_rpt( switch_mask ));
}

