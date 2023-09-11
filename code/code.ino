/* 
 *  Code last tested: 2023/08/19 ... Working! 
 *  
 *  Project Name: 3x5_touch_tft_keypad - https://github.com/phpbbireland/3x5_touch_tft_keypad
 *  
 *  An LCD/TFT (Makerfabs-ESP32-S3-SPI-TFT-with-Touch), an Image (320x240px) including 16 icons (buttons) and a Text file with 16 lines of text...
 *  
 *  Basically we simply use a predesigned 320x240 image containging 16 icons (one for each key) and map the their position to touch actions...
 *  
 *  We load the image and read the text file (menu actions) from SD card and that's it...
 *  
 *
 *  The text file consists of 15 lines, each containg the actual characters we need to send to the computer...
 *  Special keys: [ALT], [SHIFT], [CTRL], [SUPER], [SPACE], [TAB], [F1 - [F24]...
 *  
 *  Each line can contain:  
 *    A program name (can include the path) to launch, for example: ~/{HOME}/this_path/this_program
 *    Special key combinations: [ALT]{3} or [CTRL][F10] or [SHIFT][CTRL][ALT]{g}
 *    Any combination of keys up to 128 characters...
 *        
 *
 * Code relating to loading config data from file was created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-read-config-from-sd-card
 *
 * Most code from public domain, the remaining code © Michael O'Toole 2023
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

#define KEY_MAX_LENGTH    20
#define VALUE_MAX_LENGTH  128
#define FILE_NAME "/menu0"
#define TERMINAL "xfce4-terminal"
#define DEBUG 1

void remove_special_and_writekey(String str);

LGFX lcd;
USBHIDKeyboard Keyboard;  
SPIClass SD_SPI;          

int mx,my, mix = 0;
int pos[2] = {0, 0};

String menuitem01, menuitem02,menuitem03, menuitem04, menuitem05, menuitem06, menuitem07;
String menuitem08, menuitem09,menuitem10, menuitem11,menuitem12, menuitem13, menuitem14, menuitem15;

void setup(void)
{
    Serial.begin(115200);              
    //Serial.println("Keyboard begin");  
    
    lcd_init();         
    lcd.setRotation(5); // rotate so USB is on top to suit my case...

    sd_init();        

    /* Load main menu text from SD card (currently named menu0) and fill variables... */
    
    menuitem01 = SD_findString(F("menuitem01"));
    menuitem02 = SD_findString(F("menuitem02"));
    menuitem03 = SD_findString(F("menuitem03"));
    menuitem04 = SD_findString(F("menuitem04"));
    menuitem05 = SD_findString(F("menuitem05"));
    menuitem06 = SD_findString(F("menuitem06"));
    menuitem07 = SD_findString(F("menuitem07"));
    menuitem08 = SD_findString(F("menuitem08"));
    menuitem09 = SD_findString(F("menuitem09"));
    menuitem10 = SD_findString(F("menuitem10"));
    menuitem11 = SD_findString(F("menuitem11"));
    menuitem12 = SD_findString(F("menuitem12"));
    menuitem13 = SD_findString(F("menuitem13"));
    menuitem14 = SD_findString(F("menuitem14"));
    menuitem15 = SD_findString(F("menuitem15"));

    Keyboard.begin();  
    USB.begin();       

    print_img(SD, "/menu0.bmp", 480, 320);  // Load and display the Background Image 
    delay(100);
    mainMenu();
}

void loop(void)
{
   
}

/* ArduinoGetStarted.com example code Starts */

String SD_findString(const __FlashStringHelper * key)
{
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2String(value_string, value_length);
}

int SD_findKey(const __FlashStringHelper * key, char * value)
{
  File configFile = SD.open(FILE_NAME);

  if (!configFile)
  {
    Serial.print(F("SD Card: error on opening file "));
    Serial.println(FILE_NAME);
    return 0;
  }

  char key_string[KEY_MAX_LENGTH];
  char SD_buffer[KEY_MAX_LENGTH + VALUE_MAX_LENGTH + 1]; // 1 is = character
  int key_length = 0;
  int value_length = 0;

  // Flash string to string
  PGM_P keyPoiter;
  keyPoiter = reinterpret_cast<PGM_P>(key);
  byte ch;
  do {
    ch = pgm_read_byte(keyPoiter++);
    if (ch != 0)
      key_string[key_length++] = ch;
  } while (ch != 0);

  // check line by line
  while (configFile.available())
  {
    int buffer_length = configFile.readBytesUntil('\n', SD_buffer, VALUE_MAX_LENGTH+1);
    if (SD_buffer[buffer_length - 1] == '\r')
      buffer_length--; // trim the \r

    if (buffer_length > (key_length + 1)) { // 1 is = character
      if (memcmp(SD_buffer, key_string, key_length) == 0) { // equal
        if (SD_buffer[key_length] == '=') {
          value_length = buffer_length - key_length - 1;
          memcpy(value, SD_buffer + key_length + 1, value_length);
          break;
        }
      }
    }
  }

  configFile.close();  // close the file
  return value_length;
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

void  mainMenu()
{
    char str[80];

    lcd.setRotation(0);
    
    Button b[BUTTONS_PER_PAGE];

    static String b_list[BUTTONS_PER_PAGE] = {
      menuitem01,
      menuitem02,
      menuitem03,
      menuitem04,
      menuitem05,
      menuitem06,
      menuitem07,
      menuitem08,
      menuitem09,
      menuitem10,
      menuitem11,
      menuitem12,
      menuitem13,
      menuitem14,
      menuitem15
    };

    Serial.print("List of Menu Items begins... [x pos][y pos][line #]\n\n");
    
    // Build Buttons
    for (int i = 0; i < BUTTONS_PER_PAGE; i++)
    {
        Serial.print("X[");
        Serial.print(BUTTON_POS_X + i % 3 * 105);
        Serial.print("] \tY[");
        Serial.print(BUTTON_POS_Y + i / 3 * 82 + mix);
        Serial.print("] \tLine[");



        b[i].set(BUTTON_POS_X + i % 3 * 105, BUTTON_POS_Y + i / 3 * 82 + mix, 103, 95, "NULL", ENABLE);
        b[i].setText(b_list[i]);
        b[i].setValue(i);

        Serial.print(i);
        Serial.print("] ");
        
        Serial.print(" = ");
        Serial.print(b_list[i]);
        Serial.print("\n");

        

        if(i == 2 || i == 5 || i == 8 || i == 11) mix=mix+14;
        drawButton(b[i]);
    }
    Serial.print("\nList of Menu items ends...\n");

    while (1)
    {
        ft6236_pos(pos);

        for (int i = 0; i < BUTTONS_PER_PAGE; i++)
        {
            int button_value = UNABLE;
            if ((button_value = b[i].checkTouch(pos[0], pos[1])) != UNABLE)
            {
                /*
                Serial.printf("Pos is :%d,%d\n", pos[0], pos[1]);
                Serial.printf("Value is :%d\n", button_value);
                Serial.printf("Text is :");
                Serial.println(b[i].getText());
                */
                //Serial.print("{"); Serial.print(b_list[i]); Serial.print("}");
                //report();

                drawButton_p(b[i]);
                delay(BUTTON_DELAY);
                drawButton(b[i]);

                key_input_process(button_value, b_list[i]);
                //page_switch(button_value);
                delay(100);
                //lcd.enableSleep(true);
            }
        }
    }
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
/*    
    else
    {
        Serial.println("Card Mount Successed");
    }

    Serial.println("SD init over.\n\n");
*/
}

// Display image from file
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

void drawButton(Button b)
{
    int b_x;
    int b_y;
    int b_w;
    int b_h;
    int shadow_len = 3;
    String text;
    int textSize;

    b.getFoDraw(&b_x, &b_y, &b_w, &b_h, &text, &textSize);

    lcd.drawRect(b_x, b_y, b_w, b_h, COLOR_LINE);
    lcd.setCursor(b_x + 20, b_y + 20);
    lcd.setCursor(b_x + 2, b_y + b_w/2+2);
    lcd.setTextColor(COLOR_TEXT);
    lcd.setTextSize(textSize);
}

void drawButton_p(Button b)
{
    int b_x;
    int b_y;
    int b_w;
    int b_h;
    int shadow_len = 3;
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

// May add more menus later...

void page_switch(int page)
{
    switch (page)
    {
    case 0:
        //menu0();
        break;
    case 1:
        //menu1();
        break;
    case 2:
        //menu2();
        break;

    defualt:
        break;
    }
    delay(100);
}

// needs to be more robust... later

void key_input_process(int value, String str)
{
  byte isk = 0;
  byte len = str.length();
  char buffer2[129];

  for(int i = 0; i <= str.length(); i++)
  {
    buffer2[i] = str[i];
  }
  
  if(DEBUG) { Serial.print("\nThe menu line = ");  Serial.print(str);  Serial.print(" (length = "); Serial.print(len); Serial.print(")\n"); }
  
 
  if(strstr(buffer2,"[LA]"))
  {
    Keyboard.press(KEY_LEFT_ALT); isk = 1; if(DEBUG) Serial.print("\n * Left ALT was pressed *\n");
  }
  if(strstr(buffer2,"[RA]"))
  {
    Keyboard.press(KEY_RIGHT_ALT); isk = 1; if(DEBUG) Serial.print("\n * Right ALT was pressed *\n");
  }
  if(strstr(buffer2,"[LC]"))
  {
    Keyboard.press(KEY_LEFT_CTRL); isk = 1; if(DEBUG) Serial.print("\n * Left CTRL was pressed * \n");
  }
  if(strstr(buffer2,"[RC]"))
  {
    Keyboard.press(KEY_RIGHT_CTRL); isk = 1; if(DEBUG) Serial.print("\n * Right CTRL was pressed * \n");
  }
  if(strstr(buffer2,"[LS]"))
  {
    Keyboard.press(KEY_LEFT_SHIFT); isk = 1; if(DEBUG) Serial.print("\n * Left SHIFT was pressed * \n");
  }
  if(strstr(buffer2,"[RS]"))
  {
    Keyboard.press(KEY_RIGHT_SHIFT); isk = 1; if(DEBUG) Serial.print("\n * Right SHIFT was pressed * \n");
  }

  if(strstr(buffer2,"[T]"))
  {
    if(DEBUG) Serial.print("\n * Terminal launched * \n");
    Keyboard.print(TERMINAL);
    Keyboard.write(KEY_RETURN);
    delay(450);
    remove_special_and_printstr(str);
    Keyboard.write(KEY_RETURN);
    Keyboard.releaseAll();
    return;
  }
  
  //Serial.print(isk);
  
  if(isk)
  {
    remove_special_and_printstr(str);
    isk = 0;
  }
  else  
  {
    if(DEBUG) { Serial.print("No special keys in string!"); }
    Keyboard.print(str);
    delay(100);
    Keyboard.write(KEY_RETURN);
  }

  Keyboard.releaseAll();
}

void remove_special_and_printstr(String str)
{
  char buffer[129];
  
  byte i, j, k, last_bracket = 0;

  last_bracket = str.lastIndexOf(']');
  
  if(last_bracket) last_bracket++;

  for(i = last_bracket; i < (str.length()); i++)
  {
    char c = str[i];
    
    if(DEBUG)
    {
      Serial.print("\nProcessing str[i] / char c: [");
      Serial.print(c); 
      Serial.print("] Storing in: [");
      Serial.print(j);
      Serial.print("]");
    }
    
    buffer[j] = c;

    if(DEBUG)
    {
      Serial.print("\nProcessing buffer[j] = c it contains ("); Serial.print(buffer[j]); Serial.print(")");
    }
    
    j++;
  }

  buffer[j] = 0;

  if(DEBUG)
  {
    Serial.print("\nThe Keyboard.print buffer contains: [");
    Serial.print(buffer);
    Serial.print("]\t");
  }

  Keyboard.print(buffer);
}


/*
 * Show touch position in console...
 */
void report()
{
    mx = getTouchPointX();
    my = getTouchPointY();
    Serial.print("\nTouch report:");
    Serial.print("[");
    Serial.print(mx);
    Serial.print("][");
    Serial.print(my);
    Serial.print("]\n\n");
}

// Hardware init
void lcd_init()
{
    lcd.init();
    lcd.fillScreen(COLOR_BACKGROUND);

    // I2C init
    Wire.begin(I2C_SDA, I2C_SCL);
    byte error, address;
    Wire.beginTransmission(TOUCH_I2C_ADD);
    error = Wire.endTransmission();

    if (error == 0)
    {
        /*
        Serial.print("I2C device found at address 0x");
        Serial.print(TOUCH_I2C_ADD, HEX);
        Serial.println("  !");
        */
    }
    else
    {
        Serial.print("Unknown error at address 0x");
        Serial.println(TOUCH_I2C_ADD, HEX);
    }
}
