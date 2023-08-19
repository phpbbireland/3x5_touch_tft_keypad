# 3x5_touch_tft_keypad

An LCD/TFT (Makerfabs-ESP32-S3-SPI-TFT-with-Touch), an Image (320px240px) a few icons for buttons and
a Text file with 15 lines of text (these are the key presses/macros, see: sample_menu text file) and we have a macro keypad...

**Programming:**
Using Arduino IDE, compile and upload *code.ini* program to the ESP32-S3 SPI board. Install any missing libraries (see top of code.ino file for required libraries).  

**The Basics:**
We start with a 480px by 320px background image (Grid480x320.bmp), it consists of 16 button areas (identified by the red outlined squares).  
Using any image editor, we edit Grid480x320.bmp and insert/import the desired button icons (64px by 64px images works well) into each outlined button 
icons positions. Once finished, export the final image as menu0.bmp  

**Future:**
I don't currently process the menu lines for special keys, that will follow soon...  
Special keys are: [LALT], [RALT], [LSHIFT], [RSHIFT] [LCTRL], [RCTRL], [SUPER], [SPACE], [TAB], [F1 - F24].  

Each line of the menu text can contain:  
-  A program name (can include the path) to launch, for example: ~/{HOME}/this_path/this_program.  
-  Special key combinations: [ALT]{3} or [CTRL][F10] or [SHIFT][CTRL][ALT]{g}.  
-  Any combination of keys up to 128 characters.  

After initial programming of the TFT board, all changes are made to the *menu0* text file and *menu.bmp* images on the SD card.  
The changes are imported from SD card on next boot...    

*See **Grid480x320.bmp** example background image (without any icons) so you can add your own using image editing software.* 
