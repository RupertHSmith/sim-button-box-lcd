/* ruota.h

   |      |     | Signal on |                       |
   | Port | Pin | Schematic | Function              |
   |------+-----+-----------+-----------------------|
   | E    |   4 | ROTA      | Rotary Encoder A      |
   | E    |   5 | ROTB      | Rotary Encoder B      |
   | E    |   7 | SWC       | Switch wheel "Centre" |
   |------+-----+-----------+-----------------------|
   | C    |   2 | SWN       | Switch wheel "North"  |
   | C    |   3 | SWE       | Switch wheel "East"   |
   | C    |   4 | SWS       | Switch wheel "South"  |
   | C    |   5 | SWW       | Switch wheel "West"   |
   |------+-----+-----------+-----------------------|
   | B    |   6 | CD        | SD Card Detetcion     |
 
*/



#define SWN     PC2
#define SWE     PC3
#define SWS     PC4
#define SWW     PC5
#define OS_CD   PB6
#define SWC     PE7



#define REPEAT_START    60      /* after 600ms */
#define REPEAT_NEXT     10      /* every 100ms */
 
void os_init_ruota(void);

int8_t os_enc_delta(void);


uint8_t get_switch_press( uint8_t switch_mask );
uint8_t get_switch_rpt( uint8_t switch_mask );
uint8_t get_switch_state( uint8_t switch_mask );
uint8_t get_switch_short( uint8_t switch_mask );
uint8_t get_switch_long( uint8_t switch_mask );




