#include "display_controller.h"
#include "lcd.h"
#include "rios.h"
#include "ruota.h"
#include <stdlib.h>
#include <string.h>

#define NUMBER_OF_PAGES
#define DISPLAY_MODE_LAPTIMES 0
#define DISPLAY_MODE_TYRE_TEMPERATURES 1

#define MAX_MINUTES_101 6000000
#define TEN_MINUTES 600000
#define SIGN_MASK 0x8000 

#define PLUS '+'
#define MINUS '-'


bool ac_data_initialised;
bool ac_tyre_data_initialised;

uint8_t display_mode;
uint8_t current_game;

volatile ACTyreData *acTyreData;
volatile ACData *acData;

int refresh_display(int state);
int collect_switch_presses(int state);
void display_laptimes(void);
char* milli_to_string(uint32_t time);

char* delta_string(int16_t delta);
void draw_delta(int16_t delta);
void draw_laptime_current(char* str, uint32_t milli);
void draw_laptime_best(char* str, uint32_t milli);
void draw_laptime(char* best_string, int prev_length, int new_length, char* str, int y, uint32_t milli);

void receive_packet(const uint8_t* ReportData)
{
    switch (ReportData[0])
    {
        case AC_TYRE_DATA:
            process_ac_tyre_data(ReportData);
            break;
        case AC_DATA:
            process_ac_data(ReportData);
            break;
        default :
            break;        
    }
}

void process_ac_tyre_data(const uint8_t* ReportData)
{
    if (acTyreData != NULL)
    {
        current_game = ASSETTO_CORSA;

        memcpy(acTyreData, ReportData, sizeof(ACTyreData));

        if (!ac_tyre_data_initialised)
            ac_tyre_data_initialised = true;
    }    
}


void process_ac_data(const uint8_t* ReportData)
{
    if (acData != NULL){
        current_game = ASSETTO_CORSA;

        memcpy(acData, ReportData, sizeof(ACData));


        if(!ac_data_initialised)
            ac_data_initialised = true;
    }

}

void init_display_controller(void)
{
    //we will change this..
    acData = malloc(sizeof(ACData));
    acTyreData = malloc(sizeof(ACTyreData));
    display_mode = DISPLAY_MODE_LAPTIMES;

    os_add_task(refresh_display,40,1);
    os_add_task(collect_switch_presses, 80, 1);
}

int collect_switch_presses(int state)
{
    if (get_switch_press(_BV(SWW)))
    {
        
    }
    
    if (get_switch_press(_BV(SWE)))
    {
        
    }
}

int refresh_display(int state)
{
    switch(display_mode)
    {
        case DISPLAY_MODE_LAPTIMES : 
            display_laptimes();
            break;
        case DISPLAY_MODE_TYRE_TEMPERATURES :
            break;
        default:
            break;
    }
}

void init_display_laptimes(void)
{
    rectangle rect;
    rect.left = 10;
    rect.right = 310;
    rect.top = 158;
    rect.bottom = 162;
    fill_rectangle(rect, WHITE);

    display_string_font("Best", FONT_UNISPACE_14, 10, 53);
    display_string_font("Current", FONT_UNISPACE_14, 10, 125);
    clear_display_laptimes();
}

void clear_display_laptimes(void)
{
    draw_laptime_best("0:00.00", 0);
    draw_laptime_current("0:00.00", 0);
    draw_delta(0);
}

void draw_laptime_best(char* str, uint32_t milli)
{
    static char best_string[10];
    static int prev_length;
    uint8_t new_length = (milli < TEN_MINUTES) ? 7 : 8;

    draw_laptime(best_string, prev_length, new_length, str, 15, milli);

    strcpy(best_string, str);
    prev_length = new_length;
}

void draw_laptime_current(char* str, uint32_t milli)
{
    static char current_string[10];
    static int prev_length;
    uint8_t new_length = (milli < TEN_MINUTES) ? 7 : 8;

    draw_laptime(current_string, prev_length, new_length, str, 90, milli);

    strcpy(current_string, str);
    prev_length = new_length;
}


void draw_laptime(char* best_string, int prev_length, int new_length, char* str, int y, uint32_t milli)
{
    //store a copy of the current best laptime to ensure no character writes occur unnecessarily
    int x;
    char* prevPointer = best_string;

    if (prev_length == 8 && new_length == 7)
    //clear first char as wouldn't be overwritten
    {
        rectangle rect;
        rect.top = y;
        rect.bottom = y + 58;
        rect.left = 70;
        rect.right = 99;
        fill_rectangle(rect, BLACK);
        //ensure we start with an offset
        prevPointer++;
    }

    if (new_length == 7)
        x = 100;
    if (new_length == 8)
        x = 70;

    //now scan chars and if different set new char
    int ct;
    for (ct = 0; ct < new_length; ct++, prevPointer++)
    {
        if (str[ct] != *prevPointer)
        {
            //draw char
            display_char_font(str[ct], FONT_UNISPACE_36, x + (30 * ct), y);
        }
            
    }
}

void draw_delta(int16_t delta)
{
    static char prev_delta_string[10];
    static int16_t prev_delta;

    //force delta to be in range
    if (delta > 9999)
        delta = 9999;
    if (delta < -9999)
        delta = -9999;

    uint8_t new_delta_length;    
    char* new_delta_string;
    char* prevPointer = prev_delta_string;
    new_delta_string = delta_string(delta);


    uint16_t colour;
    if (SIGN_MASK & delta)
        colour = GREEN;
    else 
        colour = RED;


    int x = 130;
    int y = 170;

    if ((SIGN_MASK & prev_delta) != (SIGN_MASK & delta))
    {
        //Then we need to redraw completely as signs have changed so diff colour
        display_string_font_col(new_delta_string, FONT_UNISPACE_36, x, 170, colour);
    } else {
        //otherwise we iterate over each char and only draw those that differ
        int ct;
        for (ct = 0; ct < 6; ct++, prevPointer++)
        {
            if (new_delta_string[ct] != *prevPointer)
            {
                //draw char
                display_char_font_col(new_delta_string[ct], FONT_UNISPACE_36, x + (30 * ct), y, colour);
            }       
        }
    }

    strcpy(prev_delta_string, new_delta_string);
    prev_delta = delta;
    free(new_delta_string);
}

char* delta_string(int16_t delta)
{
    char* str = malloc(10);
    float deltaF = (float) delta / 100.f;

    dtostrf(deltaF, 6, 2, str);

    if (!(SIGN_MASK & delta))
    {
        if (str[1] == ' ')
            str[1] = PLUS;
        else 
            str[0] = PLUS;
    }
    return str;
}

void display_laptimes(void)
{
    if (current_game == ASSETTO_CORSA)
    {
        //Now display assetto corsa laptimes
        if (ac_data_initialised)
        {
            //If ac running..
            if (acData->status == AC_LIVE)
            {

                //Display best
                char* str;
                uint32_t bestTime = acData->lapTimes.bestTime;
                if (bestTime < MAX_MINUTES_101)
                {
                    str = milli_to_string(bestTime);
                    draw_laptime_best(str, bestTime);
                    free(str);
               } else 
                    draw_laptime_best("0:00.00", 0);

                //Display current
                uint32_t currentTime = acData->lapTimes.currentTime;
                if (currentTime < MAX_MINUTES_101)
                {
                    str = milli_to_string(currentTime);
                    draw_laptime_current(str, currentTime);
                    free(str);
                } else 
                    draw_laptime_current("0:00.00", 0);

                //Display delta
                draw_delta(acData->lapTimes.delta);
            } else {
                //Clear display
                clear_display_laptimes();
            }
        }
    }
}

char* milli_to_string(uint32_t time)
{
    uint16_t minute = time / 60000;
    time = time - 60000 * minute;

    uint16_t seconds = time / 1000;
    time = time - 1000 * seconds;

    uint16_t milliseconds = time / 10;

    char* str = malloc(16);
    sprintf(str, "%01u:%02u.%02u", minute, seconds, milliseconds);
    return str;
}