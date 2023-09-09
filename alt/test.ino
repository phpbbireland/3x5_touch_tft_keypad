/*
* Alternative code rework to try fix stack runout
* Last tested: 10/09/23, 10:50 it compiles!
* Reboot after 18 processes? Running out of stack...
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
/*
#define HOME 14           // Button load menu0
#define NEXT 12           // Button load next menu
#define PREV 13           // Button load previous menu
*/
#define MAXMENUS 3        // Current max menus - 1 (we start with 0)

#define TERMINAL "xfce4-terminal" // default terminal
#define DEBUG 1

LGFX lcd;
USBHIDKeyboard Keyboard;
SPIClass SD_SPI;

static uint32_t stackori = 0;
static uint32_t stacktot = 0;
static uint32_t stackrun = 0;

int _mx, _my = 0;       // global vars start with underscore _
int _pos[2] = {0, 0};   // 
int _currentMenu = 0;   //
int _selectedMenu = 0;  //
int _currentLine = 0;   // 
int _firstrun = true;   // flag to indicate program firt run

char _filename[12] = "/menu0.bmp";  //set to default menu
char _menuname[12] = "/menu0";      //set to default macro file

String b_list[BUTTONS_PER_PAGE] = { "", "", "", "", "","", "", "", "", "", "", "", "", "", "" };

Button b[BUTTONS_PER_PAGE];

char buf[128] = "0";
    
void setup(void)
{
    Serial.begin(115200);              
    lcd_init();         
    lcd.setRotation(5); // rotate so USB is on top to suit _my case...
    sd_init();
    Keyboard.begin();  
    USB.begin();       
    print_img(SD, "/logo.bmp", 480, 320);  // Load and display the Background Image 
    delay(2000);
    setFileNames(0); // begin with main menu...
}

void loop(void) { ; }

void setFileNames(int _selectedMenu)
{
    Serial.print("\n_selectedMenu = [");Serial.print(_selectedMenu);Serial.print("]");Serial.print("] _currentMenu = [");Serial.print(_currentMenu);Serial.print("]\n\n");

    if(_firstrun)
    {
      ; // continue
    }
    else if(_currentMenu == _selectedMenu)
    {
      return; // no need to process
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

    displayMenu();
    processMenu();
}

void displayMenu()
{
    lcd.setRotation(5);                   // fix for touch not rotating
    print_img(SD, _filename, 480, 320);   // Load and display the Background Image / Menu
    delay(100);  
    lcd.setRotation(0);                   // reset rotation
}

void  processMenu()
{
    char str[80];
    int adj = 0;
    processMenuFile();

    // Build Buttons
    for (int i = 0; i < BUTTONS_PER_PAGE; i++)
    {
        if(DEBUG){ Serial.print("X["); Serial.print(BUTTON_POS_X + i % 3 * 105); Serial.print("] \tY["); Serial.print(BUTTON_POS_Y + i / 3 * 82 + adj); Serial.print("] \tLine["); }

        b[i].set(BUTTON_POS_X + i % 3 * 105, BUTTON_POS_Y + i / 3 * 82 + adj, 103, 95, "NULL", ENABLE);
        b[i].setText(b_list[i]);
        b[i].setValue(i);

        if(DEBUG) { Serial.print(i); Serial.print("] "); Serial.print(" = "); Serial.print(b_list[i]); Serial.print("\n"); }
        
        if(i == 2 || i == 5 || i == 8 || i == 11) adj=adj+14; // track columns add 14px on each
        drawButton(b[i]);
    }

    //if(DEBUG) Serial.print("\nList of Menu items ends...\n");

//    ft6236_pos(pos);
//    _mx = getTouchPointX();
//    _my = getTouchPointY();
    
printStack(); // tracking stack as it will after several processes crash...
    
    //while (getTouchPointX() == _mx && getTouchPointY() == _my) // used to jump out of loop, works but is it needed?
    while(1)
    {
        ft6236_pos(_pos);
        delay(100);

        for (int i = 0; i < BUTTONS_PER_PAGE; i++)
        {
            int button_value = UNABLE;
            if ((button_value = b[i].checkTouch(_pos[0], _pos[1])) != UNABLE)
            {
                if(DEBUG) { Serial.printf("\nPos is :%d,%d\n", _pos[0], _pos[1]); Serial.printf("Value is :%d\n", button_value); Serial.printf("Text is :"); Serial.println(b[i].getText()); Serial.print("{"); Serial.print(b_list[i]); Serial.print("}"); report(); }

                drawButton_p(b[i]);
                delay(BUTTON_DELAY);
                drawButton(b[i]);
                processMenuLine(b_list[i]);
                delay(100);
            }
            //_mx = getTouchPointX();
            //_my = getTouchPointY();
        }
    }
}

void report()
{
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
        Serial.print("I2C device found at address 0x");
        Serial.print(TOUCH_I2C_ADD, HEX);
        Serial.println("  !");
    }
    else
    {
        Serial.print("Unknown error at address 0x");
        Serial.println(TOUCH_I2C_ADD, HEX);
    }
}

/*  button code from example sketch */
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

void clean_button()
{
    lcd.fillRect(BUTTON_POS_X, BUTTON_POS_Y, 319 - BUTTON_POS_X, 479 - BUTTON_POS_Y, COLOR_BACKGROUND);
}

void clean_screen()
{
    lcd.fillRect(0, 0, 319, 479, COLOR_BACKGROUND);
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




/*
    ArduinoGetStarted.com example code Starts

*/

String SD_findString(const __FlashStringHelper * token)
{
  char menu_line_text[MACRO_MAX_LENGTH];
  int macro_text_length = SD_findKey(token, menu_line_text);
  return HELPER_ascii2String(menu_line_text, macro_text_length);
}

int SD_findKey(const __FlashStringHelper * token, char * menu_line)
{
  /* why is it called for each menu line, could we not read all lines in one go? perhaps later...
  *  We have 15 lines (buttons), each having a token of [##],
  *  token[MAXMENUS][MAXMENULINES] and loop through...
  */
  
  //if(DEBUG) Serial.print("Opening: "); Serial.print(_menuname); Serial.print("\n");

  File configFile = SD.open(_menuname);

  if (!configFile)
  {
    Serial.print(F("SD Card: error on opening file "));
    Serial.println(_menuname);
    return 0;
  }

  char macro[TOKEN_MAX_LENGTH];
  char SD_buffer[TOKEN_MAX_LENGTH + MACRO_MAX_LENGTH + 1]; // 1 is = character
  int length = 0;
  int macro_text_length = 0;

  // Flash string to string
  PGM_P keyPoiter;
  keyPoiter = reinterpret_cast<PGM_P>(token);

  byte ch;

  do
  {
    ch = pgm_read_byte(keyPoiter++);
    if (ch != 0) macro[length++] = ch;
  } while (ch != 0);

  // check line by line
  while (configFile.available())
  {
    int buffer_length = configFile.readBytesUntil('\n', SD_buffer, MACRO_MAX_LENGTH+1);

    if (SD_buffer[buffer_length - 1] == '\r') buffer_length--; // trim the \r

    if (buffer_length > (length + 1)) // true for equal '=' character
    {
      if (memcmp(SD_buffer, macro, length) == 0) // if they are equal
      { 
        if (SD_buffer[length] == '=')
        {
          macro_text_length = buffer_length - length - 1;
          memcpy(menu_line, SD_buffer + length + 1, macro_text_length);
          break;
        }
      }
    }
  }

  configFile.close();  // close the file
  return macro_text_length;
}


String HELPER_ascii2String(char *ascii, int length)
{
  String str;
  str.reserve(length);
  str = "";

  for (int i = 0; i < length; i++)
  {
    char c = *(ascii + i);
    str += String(c);
  }
  return str;
}
/* ArduinoGetStarted.com example code Ends */



// Display menu image from file
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
    return 0;
}

void processMenuLine(String str)
{
    byte isk = 0;
    byte len = str.length();

    static char buffer2[MACRO_MAX_LENGTH+1];

    for (int i = 0; i <= str.length(); i++)
    {
        buffer2[i] = str[i];
    }

    if (DEBUG) { Serial.print("\nThe menu line = ");  Serial.print(str);  Serial.print(" (length = "); Serial.print(len); Serial.print(") "); }

    if (strstr(buffer2, "[RA]"))
    {
        Keyboard.press(KEY_RIGHT_ALT); isk = 1; if(DEBUG) Serial.print("\n * Right ALT was pressed *\n");
    }
    if (strstr(buffer2, "[LC]"))
    {
        Keyboard.press(KEY_LEFT_CTRL); isk = 1; if(DEBUG) Serial.print("\n * Left CTRL was pressed * \n");
    }
    if (strstr(buffer2, "[RC]"))
    {
        Keyboard.press(KEY_RIGHT_CTRL); isk = 1; if(DEBUG) Serial.print("\n * Right CTRL was pressed * \n");
    }
    if (strstr(buffer2, "[LS]"))
    {
        Keyboard.press(KEY_LEFT_SHIFT); isk = 1; if(DEBUG) Serial.print("\n * Left SHIFT was pressed * \n");
    }
    if (strstr(buffer2, "[RS]"))
    {
        Keyboard.press(KEY_RIGHT_SHIFT); isk = 1; if(DEBUG) Serial.print("\n * Right SHIFT was pressed * \n");
    }

    if (strstr(buffer2, "[T]"))
    {
        if (DEBUG) Serial.print("\n * Terminal launched * \n");
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
        if (DEBUG) { Serial.print("No special keys in string!\n"); }
        Keyboard.print(str);
        delay(100);
        Keyboard.write(KEY_RETURN);
    }

    Keyboard.releaseAll();

    if (strstr(buffer2, "[HOME]"))
    {
        _currentMenu = _selectedMenu = 0; // reset current, selected menu
        _firstrun = true;
    }
    else if (strstr(buffer2, "[NEXT]"))
    {
      if(_selectedMenu < MAXMENUS)
      {
          _currentMenu = _selectedMenu; _selectedMenu++; // update current, selected menu
      }
    }
    else if (strstr(buffer2, "[PREV]"))
    {
      if(_selectedMenu > 0)
      {
          _currentMenu = _selectedMenu; _selectedMenu--;  // update current, selected menu
      }
    }
    setFileNames(_selectedMenu);
}

void keyboard_print_macro(String str) // print menu line/macro after removing token...
{
    char buffer[129];

    int i = 0;
    int j = 0;
    int last_bracket = 0;

    last_bracket = str.lastIndexOf(']');

    if (last_bracket) last_bracket++;

    for (i = last_bracket; i < (str.length()); i++)
    {
        char c = str[i];
        if (DEBUG) { Serial.print("\nProcessing str[i] / char c: ["); Serial.print(c); Serial.print("] Storing in: ["); Serial.print(j); Serial.print("]"); }
        buffer[j] = c;
        if (DEBUG) { Serial.print("\nProcessing buffer[j] = c it contains ("); Serial.print(buffer[j]); Serial.print(")"); }
        j++;
    }

    buffer[j] = 0;
    
    if (DEBUG) { Serial.print("\nThe Keyboard.print buffer contains: ["); Serial.print(buffer); Serial.print("]\t"); }

    Keyboard.print(buffer);
}



// found on esp32,com ... para ...very useful...
void printStack(void)
{
  static int count = 0;
  
  char *SpStart = NULL;
  char *StackPtrAtStart = (char *)&SpStart;
  UBaseType_t watermarkStart = uxTaskGetStackHighWaterMark(NULL);
  char *StackPtrEnd = StackPtrAtStart - watermarkStart;
  
  if(stacktot == 0) { stackori = stacktot = (uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd; } 
  stackrun += (uint32_t)stacktot - ((uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd);
  Serial.printf("\nFree Stack near originally: %d, now: %d, ", stackori, (uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd);
  Serial.printf("used this loop[%d] = %d, Stack used, running total = %d \r\n", count++, (uint32_t)stacktot - ((uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd), stackrun);
  stacktot = (uint32_t)StackPtrAtStart - (uint32_t)StackPtrEnd;  
}

void processMenuFile(void)
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
            buf[charcount-1] = '\n';
            b_list[llcount] = buf;
            
            for(int x = 0; x < charcount; x++) buf[x] = 0;
            llcount++;
            charcount = 0;
         }
       }
       myfile.close();
    }  
    else
    {
      Serial.println("error opening the text file");
    }
}
