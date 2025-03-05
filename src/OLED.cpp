#include "ssd1306.h"
#include "ssd1306_console.h"
#include <arduino.h>


void setupOLED()
{
    /* Replace the line below with the display initialization function, you want to use */
    ssd1306_128x64_i2c_init();
    ssd1306_clearScreen();
    /* Set font to use with console */
    ssd1306_setFixedFont(ssd1306xled_font6x8);
}



void testOLED()
{
    ssd1306_print( "This is console output: " );
    ssd1306_print( "go to the next line\n" );
    delay(500);
}
