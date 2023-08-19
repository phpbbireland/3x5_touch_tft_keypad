# kissmacrokey
An LCD/TFT (Makerfabs-ESP32-S3-SPI-TFT-with-Touch), an Image (320px240px) a few icons for buttons and
a Text file with 16 lines of text (these are the key presses/macros, see: sample_menu) and we have a macro keypad...

**Programming:**
Using Arduino IDE, compile and upload code.ini program to the ESP32S3 SPI board... finished, you can change the buttons (press) actions by editing the menu0 file on the SD card, no need to reprogram...

**Basics:**
We use a 480px by 320px image (see Kiss_MacroKey.jpg for an example) containing 16 icons (one for each key). The touch positions are already programmed and are identified by the red outline squares, so, all we need to do is put some icons on the background image (using your favorite software).

**Edit background image:**
Load background image, import icon images, place them and save...

**Future**
I don't currently process the menu lines for special keys, that will follow soon.  
Special keys are: [LALT], [RALT], [LSHIFT], [RSHIFT] [LCTRL], [RCTRL], [SUPER], [SPACE], [TAB], [F1 - F24].  

Each line of the menu text can contain:  
  A program name (can include the path) to launch, for example: ~/{HOME}/this_path/this_program  
  Special key combinations: [ALT]{3} or [CTRL][F10] or [SHIFT][CTRL][ALT]{g}  
  Any combination of keys up to 128 characters...

After initial programming of the board, all you have to change things is, edit the menu0 file and/or add/remove/alter the images on the menu background image.

*I will provide a background image without icons so you can add your own using image editing software.* **Grid480x320.bmp**
