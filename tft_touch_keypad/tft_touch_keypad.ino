/*
* A rework of the code.ino to work with multiple menus/screens
* Last tested: 09/09/23, 7:50 it compiles!
* Last Edits:   Changed some var name for better readability..
* Reboot 'after x loops' Fixed, thanks to MicroController over at ESP32 Forums (infinite recursion).
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
#define LCD_SCK  14
#define LCD_CS   15
#define LCD_RST  -1 // 26
#define LCD_DC   21 // 33
#define LCD_BL   -1
#define SD_MOSI   2
#define SD_MISO  41
#define SD_SCK   42
#define SD_CS     1
#define I2C_SCL  39
#define I2C_SDA  38

#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_BUTTON     TFT_BLACK
#define COLOR_BUTTON_P   0x4BAF
#define COLOR_TEXT       TFT_WHITE
#define COLOR_LINE       TFT_RED
#define BUTTON_POS_X     2          // x margin
#define BUTTON_POS_Y     1          // y margin
#define BUTTON_DELAY     150
#define BUTTONS_PER_PAGE 15

#define TOKEN_MAX_LENGTH 5
#define MACRO_MAX_LENGTH 128
#define MAX_MENU_ITEMS   15
#define MAXMENUS         3          // Current max menus - 1 (we start with 0)

#define TERMINAL "xfce4-terminal"   // default terminal
#define DEBUG1 1
#define DEBUG2 1

LGFX lcd;
USBHIDKeyboard Keyboard;
SPIClass SD_SPI;

static uint32_t stackori = 0;
static uint32_t stacktot = 0;
static uint32_t stackrun = 0;

static int loopcount = 0;
static bool _menuchanged = true;

int _mx, _my = 0;       // global vars start with underscore _
int _pos[2] = {0, 0};   // 
int _currentMenu = 0;   //
int _selectedMenu = 0;  //
int _currentLine = 0;   // 

char _filename[12] = "/menu0.bmp";  //set to default menu
char _menuname[12] = "/menu0";      //set to default macro file

//String b_list[BUTTONS_PER_PAGE] = { "", "", "", "", "","", "", "", "", "", "", "", "", "", "" };
String b_list[BUTTONS_PER_PAGE] = { "0", "0", "0", "0", "0","0", "0", "0", "0", "0", "0", "0", "0", "0", "0" };

Button b[BUTTONS_PER_PAGE];

char buf[128] = "0";
    
void setup(void)
{
    Serial.begin(115200);
    lcd_init();
    lcd.setRotation(5); // rotate so USB is on top to suit _my case...
    sd_init();
    Keyboard.begin();
    printStack("After Kryboard begin\0");
    USB.begin();
    processMenu();
}

void loop(void) { ; }

void  processMenu()
{
    set_current_menu_filename(_selectedMenu);
    read_current_menu_file_macros_save_to_b_list();

    if(DEBUG1) { Serial.print("\nList of Menu Items begins... Buttons [x pos][y pos][line #]\n\n"); }
    
    buildButtons();

    while(1) // Welcome to the Hotel California ;)
    {
        ft6236_pos(_pos);
        delay(100);

        for (int i = 0; i < BUTTONS_PER_PAGE; i++)
        {
            int button_value = UNABLE;
            if ((button_value = b[i].checkTouch(_pos[0], _pos[1])) != UNABLE)
            {
                if(DEBUG1) { Serial.printf("\nPos is :%d, %d\n", _pos[0], _pos[1]); Serial.printf("Value is :%d\n", button_value); Serial.printf("Text is :"); Serial.println(b[i].getText()); Serial.print("{"); Serial.print(b_list[i]); Serial.print("}"); report(); }

                drawButton_p(b[i]);
                delay(BUTTON_DELAY);
                drawButton(b[i]);

                process_b_list_item_and_stuffkey_on_touch(b_list[i]);

                if(_menuchanged) // update for new Menu
                {
                    set_current_menu_filename(_selectedMenu);
                    read_current_menu_file_macros_save_to_b_list();
                    buildButtons();
                }
            }
        }
    }
    loopcount = 0;
}

void set_current_menu_filename(int _selectedMenu)
{
    if(DEBUG1) { Serial.printf("(_selectedMenu = [%d] _currentmenu = [%d])\n", _selectedMenu, _currentMenu); }

    if(_menuchanged == false)
    {
      return; // do nothing... no need to process
    }

    switch(_selectedMenu) // could just use _menuname and append ext...
    {
        case 0: { strcpy(_filename, "/menu0.bmp"); strcpy(_menuname, "/menu0"); } break;
        case 1: { strcpy(_filename, "/menu1.bmp"); strcpy(_menuname, "/menu1"); } break;
        case 2: { strcpy(_filename, "/menu2.bmp"); strcpy(_menuname, "/menu2"); } break;
        case 3: { strcpy(_filename, "/menu3.bmp"); strcpy(_menuname, "/menu3"); } break;
        //case 4: { strcpy(_filename, "/menu4.bmp"); strcpy(_menuname, "/menu4"); } break;
        //case 5: { strcpy(_filename, "/menu5.bmp"); strcpy(_menuname, "/menu5"); } break;        
        default:{ strcpy(_filename, "/menu0.bmp"); strcpy(_menuname, "/menu0"); } break;
    }

    lcd.setRotation(5);                   // fix for touch not rotating
    print_img(SD, _filename, 480, 320);   // Load and display the Background Image / Menu
    delay(100);  
    lcd.setRotation(0);                   // reset rotation
}

void read_current_menu_file_macros_save_to_b_list(void)
{
    int charcount = 0;
    int llcount = 0;
    char chr;
    
    File myfile = SD.open(_menuname);
    
    if (myfile) 
    {
      while (myfile.available())
      {
        chr =  myfile.read();
        buf[charcount++] = chr;
    
        if(chr == '\n')
        {
            buf[charcount] = '\n\r';
            b_list[llcount] = buf;
            
            for(int x = 0; x < charcount; x++) buf[x] = 0;      // Stop strange behavior...
            llcount++;
            charcount = 0;
         }
       }
       myfile.close();
    }  
    else
    {
      Serial.printf("Error opening %s menu file", _menuname);
    }
}

void process_b_list_item_and_stuffkey_on_touch(String str)
{
    byte isk = 0;                         // is special key
    byte len = str.length();

    if(DEBUG1) printStack("process_b_list_item_and_stuffkey_on_touch()");

    static char buffer2[MACRO_MAX_LENGTH+1];

    for (int i = 0; i <= str.length(); i++)
    {
        buffer2[i] = str[i]; 
    }
    ///Serial.printf("\nThe menu line is: \"%s\" (%d characters) ... " , buffer2, len);
    if (strstr(buffer2, "[LA]")) { Keyboard.press(KEY_LEFT_ALT); isk = 1; }
    if (strstr(buffer2, "[RA]")) { Keyboard.press(KEY_RIGHT_ALT); isk = 1; }
    if (strstr(buffer2, "[LC]")) { Keyboard.press(KEY_LEFT_CTRL); isk = 1; }
    if (strstr(buffer2, "[RC]")) { Keyboard.press(KEY_RIGHT_CTRL); isk = 1; }
    if (strstr(buffer2, "[LS]")) { Keyboard.press(KEY_LEFT_SHIFT); isk = 1; }
    if (strstr(buffer2, "[RS]")) { Keyboard.press(KEY_RIGHT_SHIFT); isk = 1; }

    if (strstr(buffer2, "[T]"))
    {
        Keyboard.print(TERMINAL);
        Keyboard.write(KEY_RETURN);
        delay(350);
        keyboard_print_macro(str);
        Keyboard.write(KEY_RETURN);
        Keyboard.releaseAll();
        return;
    }

    if (isk)
    {
        keyboard_print_macro(str);
        isk = 0;
    }
    else
    {
        Keyboard.print(str);
        delay(100);
        Keyboard.write(KEY_RETURN);
    }

    Keyboard.releaseAll();

    if (strstr(buffer2, "[HOME]"))
    {
      _currentMenu = _selectedMenu = 0; // reset current, selected menu
      _menuchanged = true;
    }
    else if (strstr(buffer2, "[NEXT]"))
    {
      if(_selectedMenu < MAXMENUS)
      {
        _currentMenu = _selectedMenu; _selectedMenu++; // update current, selected menu
        _menuchanged = true;
      }
    }
    else if (strstr(buffer2, "[PREV]"))
    {
      if(_selectedMenu > 0)
      {
         _currentMenu = _selectedMenu; _selectedMenu--;  // update current, selected menu
         _menuchanged = true;
      }
    }
    else
    {
      _menuchanged = false;
    }
    loopcount++;
}

void report()
{
    if(!DEBUG1) return;
    _mx = getTouchPointX();
    _my = getTouchPointY();
    Serial.print("\nTouch report:");
    Serial.print("[");
    Serial.print(_mx);
    Serial.print("][");
    Serial.print(_my);
    Serial.print("]\n\n");
}

void lcd_init()
{
    lcd.init();
    lcd.fillScreen(COLOR_BACKGROUND);

    // I2C init
    Wire.begin(I2C_SDA, I2C_SCL);
    byte error;

    Wire.beginTransmission(TOUCH_I2C_ADD);
    error = Wire.endTransmission();

    if (error == 0)
    {
      if(DEBUG1) { Serial.print("I2C device found at address 0x"); Serial.print(TOUCH_I2C_ADD, HEX); Serial.println("  !\n"); }
    }
    else
    {
        Serial.print("Unknown error at address 0x");
        Serial.println(TOUCH_I2C_ADD, HEX);
    }
}

/*  button code from example sketch */
void buildButtons()
{
    // Build Buttons red box around icon
    int adj = 0;
    for (int i = 0; i < BUTTONS_PER_PAGE; i++)
    {
        if(DEBUG1) { Serial.print("X["); Serial.printf("%3d", BUTTON_POS_X + i % 3 * 105); Serial.print("] \tY["); Serial.printf("%3d", BUTTON_POS_Y + i / 3 * 82 + adj); Serial.print("] \tLine["); }

        b[i].set(BUTTON_POS_X + i % 3 * 105, BUTTON_POS_Y + i / 3 * 82 + adj, 103, 95, "NULL", ENABLE);
        b[i].setText(b_list[i]);
        b[i].setValue(i);

        if(DEBUG1) { Serial.printf("%2d", i); Serial.print("]"); Serial.print(" = "); Serial.print(b_list[i]); Serial.print(""); }
        
        if(i == 2 || i == 5 || i == 8 || i == 11) adj=adj+14; // track columns add 14px on each
        drawButton(b[i]);
    }
    if(DEBUG1) Serial.print("\nList of Menu items ends...\n");
}

void drawButton(Button b)
{
    int b_x;
    int b_y;
    int b_w;
    int b_h;
    String text;
    int textSize;

    b.getFoDraw(&b_x, &b_y, &b_w, &b_h, &text, &textSize);

    lcd.drawRect(b_x, b_y, b_w, b_h, COLOR_LINE);
    lcd.setCursor(b_x + 20, b_y + 20);
    lcd.setCursor(b_x + 2, b_y + b_w / 2 + 2);
    lcd.setTextColor(COLOR_TEXT);
    lcd.setTextSize(textSize);
}

void drawButton_p(Button b)
{
    int b_x;
    int b_y;
    int b_w;
    int b_h;
    String text;
    int textSize;

    b.getFoDraw(&b_x, &b_y, &b_w, &b_h, &text, &textSize);

    lcd.drawRect(b_x, b_y, b_w, b_h, TFT_WHITE);
    lcd.setCursor(b_x + 20, b_y + 20);
    lcd.setTextColor(COLOR_TEXT);
    lcd.setTextSize(textSize);
}

void sd_init()
{
    SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
    if (!SD.begin(SD_CS, SD_SPI, 40000000))
    {
        Serial.println("Card Mount Failed");
        lcd.setCursor(10, 10);
        lcd.println("SD Card Failed");
        while (1)
            delay(1000);
    }
}

// Read & Display menu image from file
int print_img(fs::FS &fs, String filename, int x, int y)
{
    File f = fs.open(filename, "r");
    if (!f)
    {
        Serial.println("Failed to open file for reading");
        f.close();
        return 0;
    }

    f.seek(54);
    int X = x;
    int Y = y;
    uint8_t RGB[3 * X];
    for (int row = 0; row < Y; row++)
    {
        f.seek(54 + 3 * X * row);
        f.read(RGB, 3 * X);
        lcd.pushImage(0, row, X, 1, (lgfx::rgb888_t *)RGB);
    }
    f.close();
    return 1;
}

void keyboard_print_macro(String str) // print menu line/macro after removing token...
{
    char buffer[129] = "";

    int i = 0;
    int j = 0;
    int last_bracket = 0;

    last_bracket = str.lastIndexOf(']');

    if (last_bracket) last_bracket++;

    for (i = last_bracket; i < (str.length()); i++)
    {
        char c = str[i];
        Serial.print("\nProcessing str[i] / char c: ["); Serial.print(c); Serial.print("] Storing in: ["); Serial.print(j); Serial.print("]");
        buffer[j] = c;
        Serial.print("\nProcessing buffer[j] = c it contains ("); Serial.print(buffer[j]); Serial.print(")");
        j++;
    }

    buffer[j] = 0;
    if(DEBUG1) Serial.printf("\nThe Keyboard.print buffer contains: [%s]", buffer); 
    Keyboard.print(buffer);
}

// found on esp32,com ... modified ...very useful...
void printStack(char *mytxt)
{

  char *SpStart = NULL;
  char *StackPtrAtStart = (char *)&SpStart;
  
  UBaseType_t watermarkStart = uxTaskGetStackHighWaterMark(NULL);
  
  char *StackPtrEnd = StackPtrAtStart - watermarkStart;
  
  if(stacktot == 0) { stackori = stacktot = (uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd; } 
  stackrun += (uint32_t)stacktot - ((uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd);
  
  Serial.printf("Free Stack near previous: %d, now: %d,", stackori, (uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd);
  Serial.printf(" this loop [%d] used: %4d, total stack used:%4d (%s)\r\n", loopcount, (uint32_t)stacktot - ((uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd), stackrun, mytxt);
  
  stacktot = (uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd;
}

