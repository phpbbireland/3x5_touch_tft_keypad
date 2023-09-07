/*
* A rework of the code to work with mulitple menus/screens
*
*/

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <vector>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "FT6236.h"
#include "Button.h"
#include "SPI_9488.h"

#define LCD_MOSI 13
#define LCD_MISO 12
#define LCD_SCK 14
#define LCD_CS 15
#define LCD_RST -1 // 26
#define LCD_DC 21  // 33
#define LCD_BL -1
#define SD_MOSI 2
#define SD_MISO 41
#define SD_SCK 42
#define SD_CS 1
#define I2C_SCL 39
#define I2C_SDA 38

#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_BUTTON TFT_BLACK
#define COLOR_BUTTON_P 0x4BAF
#define COLOR_TEXT TFT_WHITE
#define COLOR_LINE TFT_RED
#define BUTTON_POS_X 2         // button x margin
#define BUTTON_POS_Y 1         // button y margin
#define BUTTON_DELAY 150
#define BUTTONS_PER_PAGE 15

int _currentMenu = 0;
int _selectedMenu = 0;

void setup(void)
{
    Serial.begin(115200);              
    lcd_init();         
    lcd.setRotation(5); // rotate so USB is on top to suit my case...
    sd_init();
    Keyboard.begin();  
    USB.begin();       
    print_img(SD, "/logo.bmp", 480, 320);  // Load and display the Background Image 
    delay(100);
    getTouch();
}

void setFileNames(int _selectedMenu)
{
    if(_selectedMenu == 0)      { strcpy(filename, "/menu0.bmp"); strcpy(menuname, "/menu0"); }
    else if(_selectedMenu == 1) { strcpy(filename, "/menu1.bmp"); strcpy(menuname, "/menu1"); }
    else if(_selectedMenu == 2) { strcpy(filename, "/menu2.bmp"); strcpy(menuname, "/menu2"); }
    else if(_selectedMenu == 3) { strcpy(filename, "/menu3.bmp"); strcpy(menuname, "/menu3"); }

    displayMenus();
    processMenu();
}

void displayMenu()
{
    print_img(SD, filename, 480, 320);  // Load and display the Background Image / Menu
    delay(100);  
}

void processMenu(void)
{
    for (int i = 0; i < BUTTONS_PER_PAGE; i++)
    {
      int button_value = UNABLE;
      if ((button_value = b[i].checkTouch(pos[0], pos[1])) != UNABLE)
      {
          if(DEBUG) { Serial.printf("Pos is :%d,%d\n", pos[0], pos[1]); Serial.printf("Value is :%d\n", button_value); Serial.printf("Text is :"); Serial.println(b[i].getText()); Serial.print("{"); Serial.print(b_list[i]); Serial.print("}"); report(); }
          drawButton_p(b[i]);
          delay(BUTTON_DELAY);
          drawButton(b[i]);
          key_input_process(button_value, b_list[i]);
      }
    }
}

void getTouch(void)
{
    while (1)
    {
        ft6236_pos(pos);
    }  
}



