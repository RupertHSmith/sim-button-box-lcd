/*  Author: Steve Gunn
 * Licence: This work is licensed under the Creative Commons Attribution License.
 *           View this license at http://creativecommons.org/about/licenses/
 *
 *  
 *  - Jan 2015  Modified for LaFortuna (Rev A, black edition) [KPZ]
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "font.h"
#include "unispace_font.h"
#include "ili934x.h"
#include "lcd.h"

lcd display;

void init_lcd()
{
    /* Enable extended memory interface with 10 bit addressing */
    XMCRB = _BV(XMM2) | _BV(XMM1);
    XMCRA = _BV(SRE);
    DDRC |= _BV(RESET);
    DDRB |= _BV(BLC);
    _delay_ms(1);
    PORTC &= ~_BV(RESET);
    _delay_ms(20);
    PORTC |= _BV(RESET);
    _delay_ms(120);
    write_cmd(DISPLAY_OFF);
    write_cmd(SLEEP_OUT);
    _delay_ms(60);
    write_cmd_data(INTERNAL_IC_SETTING,          0x01);
    write_cmd(POWER_CONTROL_1);
        write_data16(0x2608);
    write_cmd_data(POWER_CONTROL_2,              0x10);
    write_cmd(VCOM_CONTROL_1);
        write_data16(0x353E);
    write_cmd_data(VCOM_CONTROL_2, 0xB5);
    write_cmd_data(INTERFACE_CONTROL, 0x01);
        write_data16(0x0000);
    write_cmd_data(PIXEL_FORMAT_SET, 0x55);     /* 16bit/pixel */
    set_orientation(West);
    clear_screen();
    display.x = 0;
    display.y = 0;
    display.background = BLACK;
    display.foreground = WHITE;
    write_cmd(DISPLAY_ON);
    _delay_ms(50);
    write_cmd_data(TEARING_EFFECT_LINE_ON, 0x00);
    EICRB |= _BV(ISC61);
    PORTB |= _BV(BLC);
}

void lcd_brightness(uint8_t i)
{
    /* Configure Timer 2 Fast PWM Mode 3 */
    TCCR2A = _BV(COM2A1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(CS20);
    OCR2A = i;
}

void set_orientation(orientation o)
{
    display.orient = o;
    write_cmd(MEMORY_ACCESS_CONTROL);
    if (o==North) { 
        display.width = LCDWIDTH;
        display.height = LCDHEIGHT;
        write_data(0x48);
    }
    else if (o==West) {
        display.width = LCDHEIGHT;
        display.height = LCDWIDTH;
        write_data(0xE8);
    }
    else if (o==South) {
        display.width = LCDWIDTH;
        display.height = LCDHEIGHT;
        write_data(0x88);
    }
    else if (o==East) {
        display.width = LCDHEIGHT;
        display.height = LCDWIDTH;
        write_data(0x28);
    }
    write_cmd(COLUMN_ADDRESS_SET);
    write_data16(0);
    write_data16(display.width-1);
    write_cmd(PAGE_ADDRESS_SET);
    write_data16(0);
    write_data16(display.height-1);
}



void set_frame_rate_hz(uint8_t f)
{
    uint8_t diva, rtna, period;
    if (f>118)
        f = 118;
    if (f<8)
        f = 8;
    if (f>60) {
        diva = 0x00;
    } else if (f>30) {
        diva = 0x01;
    } else if (f>15) {
        diva = 0x02;
    } else {
        diva = 0x03;
    }
    /*   !!! FIXME !!!  [KPZ-30.01.2015] */
    /*   Check whether this works for diva > 0  */
    /*   See ILI9341 datasheet, page 155  */
    period = 1920.0/f;
    rtna = period >> diva;
    write_cmd(FRAME_CONTROL_IN_NORMAL_MODE);
    write_data(diva);
    write_data(rtna);
}

void fill_rectangle(rectangle r, uint16_t col)
{
    write_cmd(COLUMN_ADDRESS_SET);
    write_data16(r.left);
    write_data16(r.right);
    write_cmd(PAGE_ADDRESS_SET);
    write_data16(r.top);
    write_data16(r.bottom);
    write_cmd(MEMORY_WRITE);
/*  uint16_t x, y;
    for(x=r.left; x<=r.right; x++)
        for(y=r.top; y<=r.bottom; y++)
            write_data16(col);
*/
    uint16_t wpixels = r.right - r.left + 1;
    uint16_t hpixels = r.bottom - r.top + 1;
    uint8_t mod8, div8;
    uint16_t odm8, odd8;
    if (hpixels > wpixels) {
        mod8 = hpixels & 0x07;
        div8 = hpixels >> 3;
        odm8 = wpixels*mod8;
        odd8 = wpixels*div8;
    } else {
        mod8 = wpixels & 0x07;
        div8 = wpixels >> 3;
        odm8 = hpixels*mod8;
        odd8 = hpixels*div8;
    }
    uint8_t pix1 = odm8 & 0x07;
    while(pix1--)
        write_data16(col);

    uint16_t pix8 = odd8 + (odm8 >> 3);
    while(pix8--) {
        write_data16(col);
        write_data16(col);
        write_data16(col);
        write_data16(col);
        write_data16(col);
        write_data16(col);
        write_data16(col);
        write_data16(col);
    }
}

void fill_rectangle_indexed(rectangle r, uint16_t* col)
{
    uint16_t x, y;
    write_cmd(COLUMN_ADDRESS_SET);
    write_data16(r.left);
    write_data16(r.right);
    write_cmd(PAGE_ADDRESS_SET);
    write_data16(r.top);
    write_data16(r.bottom);
    write_cmd(MEMORY_WRITE);
    for(x=r.left; x<=r.right; x++)
        for(y=r.top; y<=r.bottom; y++)
            write_data16(*col++);
}

void clear_screen()
{
    display.x = 0;
    display.y = 0;
    rectangle r = {0, display.width-1, 0, display.height-1};
    fill_rectangle(r, display.background);
}

void display_char(char c)
{
    uint16_t x, y;
    PGM_P fdata; 
    uint8_t bits, mask;
    uint16_t sc=display.x, ec=display.x + 4, sp=display.y, ep=display.y + 7;

    /*   New line starts a new line, or if the end of the
         display has been reached, clears the display.
    */
    if (c == '\n') { 
        display.x=0; display.y+=8;
        if (display.y >= display.height) { clear_screen(); }
        return;
    }

    if (c < 32 || c > 126) return;
    fdata = (c - ' ')*5 + font5x7;
    write_cmd(PAGE_ADDRESS_SET);
    write_data16(sp);
    write_data16(ep);
    for(x=sc; x<=ec; x++) {
        write_cmd(COLUMN_ADDRESS_SET);
        write_data16(x);
        write_data16(x);
        write_cmd(MEMORY_WRITE);
        bits = pgm_read_byte(fdata++);
        for(y=sp, mask=0x01; y<=ep; y++, mask<<=1)
            write_data16((bits & mask) ? display.foreground : display.background);
    }
    write_cmd(COLUMN_ADDRESS_SET);
    write_data16(x);
    write_data16(x);
    write_cmd(MEMORY_WRITE);
    for(y=sp; y<=ep; y++)
        write_data16(display.background);

    display.x += 6;
    if (display.x >= display.width) { display.x=0; display.y+=8; }
}

void display_string(char *str)
{
    uint8_t i;
    for(i=0; str[i]; i++) 
        display_char(str[i]);
}

void display_string_xy(char *str, uint16_t x, uint16_t y)
{
    uint8_t i;
    display.x = x;
    display.y = y;
    for(i=0; str[i]; i++)
        display_char(str[i]);
}

void display_register(uint8_t reg)
{
	uint8_t i;

	for(i = 0; i < 8; i++) {
		if((reg & (_BV(7) >> i)) != 0) {
			display_char( '1' );
		} else {
			display_char( '.' );
		}
	}
}

int display_string_font(char *str, uint8_t font, uint16_t x, uint16_t y)
{
    return display_string_font_col(str, font, x, y, display.foreground);
}

int display_string_font_col(char *str, uint8_t font, uint16_t x, uint16_t y, uint16_t col)
{
    switch (font)
    {
        case FONT_UNISPACE_36 :
            {
                uint8_t i;
                for(i=0; str[i]; i++)
                {
                    display_char_font_col(str[i], font, x + (30 * i), y, col);
                }
            }
            return FONT_WRITE_SUCCESS;
        case FONT_UNISPACE_18:
            {
                uint8_t i;
                for (i=0; str[i];i++)
                {
                    display_char_font_col(str[i], font, x+(16*i), y, col);
                }
            }
            return FONT_WRITE_SUCCESS;
        case FONT_UNISPACE_ITALIC_18:
            {
                uint8_t i;
                for (i=0; str[i];i++)
                {
                    display_char_font_col(str[i], font, x+(15*i), y, col);
                }
            }
            return FONT_WRITE_SUCCESS;
        case FONT_UNISPACE_14 : 
            {
                uint8_t i;
                for(i=0; str[i]; i++)
                {
                    display_char_font_col(str[i], font, x + (7 * i), y, col);
                }
            }
        default : 
            return ERR_NO_SUCH_FONT;
    }
}


int display_string_font_len(char *str, uint8_t length, uint8_t font, uint16_t x, uint16_t y)
{
    switch (font)
    {
        case FONT_UNISPACE_36 :
            {
                uint8_t i;
                for(i=0; i<length; i++)
                {
                    display_char_font(str[i], font, x + (30 * i), y);
                }
            }
            return FONT_WRITE_SUCCESS;
        case FONT_UNISPACE_14 : 
            {
                uint8_t i;
                for(i=0; i<length; i++)
                {
                    display_char_font(str[i], font, x + (7 * i), y);
                }
            }
        default : 
            return ERR_NO_SUCH_FONT;
    }
}

int display_char_font(char c, uint8_t font, uint16_t x, uint16_t y)
{
    display_char_font_col(c, font, x, y, display.foreground);
}

int display_char_font_col(char c, uint8_t font, uint16_t x, uint16_t y, uint16_t col)
{
    if (c < 32 || c > 127) return ERR_CHAR_OUT_OF_RANGE;

    uint8_t characterIndex = c - 32;




    switch (font)
    {
        case FONT_UNISPACE_14 :
            {
                write_cmd(PAGE_ADDRESS_SET);
                write_data16(y);
                write_data16(y + 14);
                //Print char at loc
                int bytesPerChar = 15;

                int horizontalSize = 7;
                int verticalSize = 14;

                // Pointer to start of character data (+ 1 since the first byte is ignored) 
                PGM_P fdata = Unispace7x14 + bytesPerChar * characterIndex + 1;

                int writePos;
                for (writePos = x; writePos < (x + 7); writePos++)
                {

                    write_cmd(COLUMN_ADDRESS_SET);
                    write_data16(writePos);
                    write_data16(writePos);
                    write_cmd(MEMORY_WRITE);

                    //contains column of bytes where when read right to left reads top to bottom (ignore 2 most significant bits)
                    uint16_t reorderedColumnBytes = pgm_read_byte(fdata++);
                    reorderedColumnBytes |= (pgm_read_byte(fdata++) << 8);

                    //now print this column
                    int ct;
                    uint16_t mask = 0x0001;
                    for (ct=0; ct<14; ct++)
                    {
                        write_data16((reorderedColumnBytes & mask) ? col : display.background);
                        mask <<= 1;
                    }
                    
                }
            }
            break;
        case FONT_UNISPACE_18:
            {
               //Print char at loc
                // 4 bytes per column, 15 columns, + 1 ignored byte
                uint8_t bytesPerChar = 61;
                uint8_t bytesPerColumn = 4;

                uint8_t horizontalSize = 15;
                uint8_t verticalSize = 29;

                write_cmd(PAGE_ADDRESS_SET);
                write_data16(y);
                write_data16(y + verticalSize);

                // Pointer to start of character data (+ 1 since the first byte is ignored) 
                PGM_P fdata = Unispace15x29 + bytesPerChar * characterIndex + 1;

                uint16_t writePos;
                for (writePos = x; writePos < (x + horizontalSize); writePos++)
                {

                    write_cmd(COLUMN_ADDRESS_SET);
                    write_data16(writePos);
                    write_data16(writePos);
                    write_cmd(MEMORY_WRITE);


                    int ct0;
                    for (ct0 = 0; ct0 < bytesPerColumn; ct0++)
                    {
                        
                        //contains column of bytes where when read right to left reads top to bottom
                        uint8_t columnBytes = pgm_read_byte(fdata++);

                        //now print this column
                        int ct;
                        uint8_t mask = 0x01;
                        for (ct=0; ct<8; ct++)
                        {
                            if ((ct0 * 8 + ct) < verticalSize)
                            {
                                write_data16((columnBytes & mask) ? col : display.background);
                                mask <<= 1;
                            }                             
                        }
                    }
                    
                }
            }
            break;
        case FONT_UNISPACE_ITALIC_18:
            {
               //Print char at loc
                // 4 bytes per column, 15 columns, + 1 ignored byte
                uint8_t bytesPerChar = 61;
                uint8_t bytesPerColumn = 4;

                uint8_t horizontalSize = 15;
                uint8_t verticalSize = 29;

                write_cmd(PAGE_ADDRESS_SET);
                write_data16(y);
                write_data16(y + verticalSize);

                // Pointer to start of character data (+ 1 since the first byte is ignored) 
                PGM_P fdata = UnispaceItalic15x29 + bytesPerChar * characterIndex + 1;

                uint16_t writePos;
                for (writePos = x; writePos < (x + horizontalSize); writePos++)
                {

                    write_cmd(COLUMN_ADDRESS_SET);
                    write_data16(writePos);
                    write_data16(writePos);
                    write_cmd(MEMORY_WRITE);


                    int ct0;
                    for (ct0 = 0; ct0 < bytesPerColumn; ct0++)
                    {
                        
                        //contains column of bytes where when read right to left reads top to bottom
                        uint8_t columnBytes = pgm_read_byte(fdata++);

                        //now print this column
                        int ct;
                        uint8_t mask = 0x01;
                        for (ct=0; ct<8; ct++)
                        {
                            if ((ct0 * 8 + ct) < verticalSize)
                            {
                                write_data16((columnBytes & mask) ? col : display.background);
                                mask <<= 1;
                            }                             
                        }
                    }
                    
                }
            }
            break;
        case FONT_UNISPACE_36 :
            {

                //Print char at loc
                // 8 bytes per column, 29 columns, + 1 ignored byte
                uint8_t bytesPerChar = 233;
                uint8_t bytesPerColumn = 8;

                uint8_t horizontalSize = 29;
                uint8_t verticalSize = 58;

                write_cmd(PAGE_ADDRESS_SET);
                write_data16(y);
                write_data16(y + verticalSize);

                // Pointer to start of character data (+ 1 since the first byte is ignored) 
                PGM_P fdata = Unispace29x58 + bytesPerChar * characterIndex + 1;

                uint16_t writePos;
                for (writePos = x; writePos < (x + horizontalSize); writePos++)
                {

                    write_cmd(COLUMN_ADDRESS_SET);
                    write_data16(writePos);
                    write_data16(writePos);
                    write_cmd(MEMORY_WRITE);


                    int ct0;
                    for (ct0 = 0; ct0 < bytesPerColumn; ct0++)
                    {
                        
                        //contains column of bytes where when read right to left reads top to bottom
                        uint8_t columnBytes = pgm_read_byte(fdata++);

                        //now print this column
                        int ct;
                        uint8_t mask = 0x01;
                        for (ct=0; ct<8; ct++)
                        {
                            if ((ct0 * 8 + ct) < verticalSize)
                            {
                                write_data16((columnBytes & mask) ? col : display.background);
                                mask <<= 1;
                            }                             
                        }
                    }
                    
                }
            }
            break;
        
        default :
            return ERR_NO_SUCH_FONT;
    }
    return FONT_WRITE_SUCCESS;
}

