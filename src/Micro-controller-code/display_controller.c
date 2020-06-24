#include "display_controller.h"
#include "lcd.h"
#include "rios.h"
#include "ruota.h"
#include <stdlib.h>
#include <string.h>

#define NUMBER_OF_PAGES 3
#define DISPLAY_MODE_BLANK 0
#define AC_DISPLAY_MODE_LAPTIMES 1
#define AC_DISPLAY_MODE_TYRE_TEMPERATURES 2

#define OPTIMUM_TYRE_TEMPERATURE 85
#define MAX_TYRE_TEMP 110
#define MIN_TYRE_TEMP 20

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

const rectangle tyre_FL = { 90, 148, 14, 108 }; 
const rectangle tyre_FR = { 172, 230, 14, 108 };
const rectangle tyre_RL = { 90, 148, 132, 226 };
const rectangle tyre_RR = { 172, 230, 132, 226 };

int refresh_display(int state);
int collect_switch_presses(int state);
void display_laptimes(void);
char* milli_to_string(uint32_t time);

char* delta_string(int16_t delta);
void draw_delta(int16_t delta, bool forceDraw);
void draw_laptime_current(char* str, uint32_t milli, bool forceDraw);
void draw_laptime_best(char* str, uint32_t milli, bool forceDraw);
void draw_laptime(char* best_string, int prev_length, int new_length, char* str, int y, uint32_t milli, bool forceDraw);
void init_tyre_temps_display(void);
void set_tyre_temperatures(TyreTemp *tyreTemp);
uint16_t calculate_tyre_colour(uint16_t temperature);
void set_tyre_wear(TyreWear *tyreWear);
void set_tyre_pressure(TyrePressure *tyrePressure);
void init_tyre_wear(void);
void init_tyre_temps(void);

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

    os_add_task(refresh_display,40,1);
    os_add_task(collect_switch_presses, 80, 1);
}

int collect_switch_presses(int state)
{
    if (get_switch_press(_BV(SWW)))
    {
        if (display_mode > 0)
            display_mode--;
    }
    
    if (get_switch_press(_BV(SWE)))
    {
        //only change page if we're not on final page
        if (display_mode < (NUMBER_OF_PAGES - 1))
            display_mode++;
    }
}

int refresh_display(int state)
{
    //Check if display mode has changed
    static uint8_t current_display_mode;
    if (display_mode != current_display_mode)
    {
        //Then we need to switch displays
        switch (display_mode)
        {
            case DISPLAY_MODE_BLANK:
                clear_screen();
                char str[20];
                sprintf(str, "Size of ACData: %d\nSize of ACTyreData: %d\n", sizeof(ACData), sizeof(ACTyreData));
                display_string(str);
                break;
            case AC_DISPLAY_MODE_LAPTIMES: 
                init_display_laptimes();
                break;
            case AC_DISPLAY_MODE_TYRE_TEMPERATURES: 
                init_tyre_temps_display();
                break;
            default : 
                break;
        }


        //Finally update current_display_mode to reflect change
        current_display_mode = display_mode;
    }

    //Display is in correct mode so now display data
    switch(display_mode)
    {
        case AC_DISPLAY_MODE_LAPTIMES : 
            display_laptimes();
            break;
        case AC_DISPLAY_MODE_TYRE_TEMPERATURES :
            break;
        default:
            break;
    }
}

void init_tyre_temps_display(void)
{
    clear_screen();
    init_tyre_wear();
    init_tyre_temps();


    //demo draw
    TyreTemp tyreTemp = {80, 44, 105,88};
    TyreWear tyreWear = {58, 100, 70, 100};
    TyreWear tyreWear2 = {59, 44, 15, 32};

    set_tyre_wear(&tyreWear);
    set_tyre_temperatures(&tyreTemp);
    set_tyre_wear(&tyreWear2);
}

void init_tyre_wear(void)
{
    TyreWear tyreWear = {0,0,0,0};
    set_tyre_wear(&tyreWear);
}

void init_tyre_temps(void)
{
    TyreTemp tyreTemp = {0,0,0,0};
    set_tyre_temperatures(&tyreTemp);
}

void set_tyre_wear(TyreWear *tyreWear)
{
    //all 15px from tyre block
    static TyreWear currentTyreWear;

    uint16_t tyreWearFL = tyreWear->frontL;
    uint16_t tyreWearFR = tyreWear->frontR;
    uint16_t tyreWearRL = tyreWear->rearL;
    uint16_t tyreWearRR = tyreWear->rearR;

    //Front left
    if (currentTyreWear.frontL != tyreWearFL)
    {
        char tyreWearStr[4];
        char paddedTyreStr[4];

        sprintf(tyreWearStr, "%u%%", tyreWearFL);
        sprintf(paddedTyreStr, "%4s", tyreWearStr);
        display_string_font(paddedTyreStr, FONT_UNISPACE_ITALIC_18, 15, 26);
        currentTyreWear.frontL = tyreWearFL;
    }

    //Front right
    if (currentTyreWear.frontR != tyreWearFR)
    {
        char tyreWearStr[4];

        sprintf(tyreWearStr, "%u%%", tyreWearFR);
        display_string_font(tyreWearStr, FONT_UNISPACE_ITALIC_18, 245, 26);
        currentTyreWear.frontR = tyreWearFR;
    }
    //if prev was a 3 char string (<100) then we need to clear the last char
    if (currentTyreWear.frontR >= 100)
    {
        //Then clear the char
        rectangle rect = { 290, 305, 26, 55 };
        fill_rectangle(rect, BLACK);
    }

    //Rear left
    if (currentTyreWear.rearL != tyreWearRL)
    {
        char tyreWearStr[4];
        char paddedTyreStr[4];

        sprintf(tyreWearStr, "%u%%", tyreWearRL);
        sprintf(paddedTyreStr, "%4s", tyreWearStr);
        display_string_font(paddedTyreStr, FONT_UNISPACE_ITALIC_18, 15, 144);
        currentTyreWear.rearL = tyreWearRL;
    }

    //Rear right
    if (currentTyreWear.rearR != tyreWearRR)
    {
        char tyreWearStr[10];

        sprintf(tyreWearStr, "%u%%", tyreWearRR);
        display_string_font(tyreWearStr, FONT_UNISPACE_ITALIC_18, 245, 144);
        currentTyreWear.rearR = tyreWearRR;
    }
    //if prev was a 3 char string (<100) then we need to clear the last char
    if (currentTyreWear.rearR >= 100)
    {
        //Then clear the char
        rectangle rect = { 290, 305, 144, 173 };
        fill_rectangle(rect, BLACK);
    }
}

void set_tyre_temperatures(TyreTemp *tyreTemp)
{
    static TyreTemp currentTyreTempDisplay;

    //Front left
    if (tyreTemp->frontL != currentTyreTempDisplay.frontL)
    {
        uint16_t tyreColour = calculate_tyre_colour(tyreTemp->frontL);
        fill_rectangle(tyre_FL, tyreColour);

        display.background = tyreColour;

        char tyreTempStr[3];
        uint16_t frontLTyreTemp = tyreTemp->frontL;
        sprintf(tyreTempStr, "%u", frontLTyreTemp);

        if (tyreTemp < 100)
            display_string_font_col(tyreTempStr,FONT_UNISPACE_ITALIC_18, 104, 47, BLACK);
        else
            display_string_font_col(tyreTempStr,FONT_UNISPACE_ITALIC_18, 96, 47, BLACK);
        display.background = BLACK;
        currentTyreTempDisplay.frontL = frontLTyreTemp;
    }

    //Front right
    if (tyreTemp->frontR != currentTyreTempDisplay.frontR)
    {
        uint16_t tyreColour = calculate_tyre_colour(tyreTemp->frontR);
        fill_rectangle(tyre_FR, tyreColour);

        display.background = tyreColour;

        char tyreTempStr[3];
        uint16_t frontRTyreTemp = tyreTemp->frontR;
        sprintf(tyreTempStr, "%u", frontRTyreTemp);

        if (tyreTemp < 100)
            display_string_font_col(tyreTempStr,FONT_UNISPACE_ITALIC_18, 186, 47, BLACK);
        else
            display_string_font_col(tyreTempStr,FONT_UNISPACE_ITALIC_18, 178, 47, BLACK);
        display.background = BLACK;
        currentTyreTempDisplay.frontR = frontRTyreTemp;
    }

    //Rear left
    if (tyreTemp->rearL != currentTyreTempDisplay.rearL)
    {
        uint16_t tyreColour = calculate_tyre_colour(tyreTemp->rearL);
        fill_rectangle(tyre_RL, tyreColour);

        display.background = tyreColour;

        char tyreTempStr[4];
        uint16_t rearLTyreTemp = tyreTemp->rearL;
        sprintf(tyreTempStr, "%u", rearLTyreTemp);

       if (tyreTemp < 100)
            display_string_font_col(tyreTempStr,FONT_UNISPACE_ITALIC_18, 104, 165, BLACK);
        else
            display_string_font_col(tyreTempStr,FONT_UNISPACE_ITALIC_18, 96, 165, BLACK);
        display.background = BLACK;
        currentTyreTempDisplay.rearL = rearLTyreTemp;
    }

    //Rear right
    if (tyreTemp->rearR != currentTyreTempDisplay.rearR)
    {
        uint16_t tyreColour = calculate_tyre_colour(tyreTemp->rearR);
        fill_rectangle(tyre_RR, tyreColour);

        display.background = tyreColour;

        char tyreTempStr[3];
        uint16_t rearRTyreTemp = tyreTemp->rearR;
        sprintf(tyreTempStr, "%u",tyreTemp->rearR);

        if (tyreTemp < 100)
            display_string_font_col(tyreTempStr,FONT_UNISPACE_ITALIC_18, 186, 165, BLACK);
        else
            display_string_font_col(tyreTempStr,FONT_UNISPACE_ITALIC_18, 178, 165, BLACK);
        display.background = BLACK;
        currentTyreTempDisplay.rearR = tyreTemp->rearR;
    }
}

uint16_t calculate_tyre_colour(uint16_t temperature)
{
    uint16_t rVal;
    uint16_t bVal;
    uint16_t gVal;

    float optToMaxRatio = (float) (temperature - OPTIMUM_TYRE_TEMPERATURE) / (float) (MAX_TYRE_TEMP - OPTIMUM_TYRE_TEMPERATURE);
    float minToOptRatio = (float) (temperature - MIN_TYRE_TEMP) / (float) (OPTIMUM_TYRE_TEMPERATURE - MIN_TYRE_TEMP);

    //Red value
    if (temperature >= MAX_TYRE_TEMP)
        rVal = 0x1F << 11;
    else if (temperature >= OPTIMUM_TYRE_TEMPERATURE)
    {
        rVal = (uint16_t) (optToMaxRatio * 0x1F);
        rVal <<= 11;
    }
    else
    {
        rVal = 0;
    }

    //Green value
    if (temperature >= OPTIMUM_TYRE_TEMPERATURE && temperature <= MAX_TYRE_TEMP)
    {
        //Then g value is full at optimum, zero at max
        gVal = (uint16_t) ((1 - optToMaxRatio) * 0x3F);
        gVal <<= 5;
    }
    else if (temperature < OPTIMUM_TYRE_TEMPERATURE && temperature >= MIN_TYRE_TEMP)
    {
        gVal = (uint16_t) (minToOptRatio * 0x3F);
        gVal <<= 5;
    }
    else
    {
        gVal = 0;
    }

    //Blue value
    if (temperature <= MIN_TYRE_TEMP)
        bVal = 0x1F;
    else if (temperature < OPTIMUM_TYRE_TEMPERATURE)
    {
        bVal = (uint16_t) ((1 - minToOptRatio) * 0x1F);
    }
    else
    {
        bVal = 0;
    }
    return rVal | gVal | bVal;
}

void init_display_laptimes(void)
{
    clear_screen();
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
    draw_laptime_best("0:00.00", 0, true);
    draw_laptime_current("0:00.00", 0, true);
    draw_delta(0, true);
}

void draw_laptime_best(char* str, uint32_t milli, bool forceDraw)
{
    static char best_string[10];
    static int prev_length;
    uint8_t new_length = (milli < TEN_MINUTES) ? 7 : 8;

    draw_laptime(best_string, prev_length, new_length, str, 15, milli, forceDraw);

    strcpy(best_string, str);
    prev_length = new_length;
}

void draw_laptime_current(char* str, uint32_t milli, bool forceDraw)
{
    static char current_string[10];
    static int prev_length;
    uint8_t new_length = (milli < TEN_MINUTES) ? 7 : 8;

    draw_laptime(current_string, prev_length, new_length, str, 90, milli, forceDraw);

    strcpy(current_string, str);
    prev_length = new_length;
}


void draw_laptime(char* best_string, int prev_length, int new_length, char* str, int y, uint32_t milli, bool forceDraw)
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

    //If forceDraw then we must just draw regardless
    if (forceDraw)
    {
        display_string_font(str, FONT_UNISPACE_36,x,y);
    }
    else
    {
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
}

void draw_delta(int16_t delta, bool forceDraw)
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

    if ((SIGN_MASK & prev_delta) != (SIGN_MASK & delta) || forceDraw)
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
                    draw_laptime_best(str, bestTime, false);
                    free(str);
               } else 
                    draw_laptime_best("0:00.00", 0, false);

                //Display current
                uint32_t currentTime = acData->lapTimes.currentTime;
                if (currentTime < MAX_MINUTES_101)
                {
                    str = milli_to_string(currentTime);
                    draw_laptime_current(str, currentTime, false);
                    free(str);
                } else 
                    draw_laptime_current("0:00.00", 0, false);

                //Display delta
                draw_delta(acData->lapTimes.delta, false);
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