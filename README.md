# 3x5_touch_tft_keypad

<br>
<img src="/images/imagebuild.png" width=480>  

Example of finished project (stl files here) [Thinyverse](https://www.thingiverse.com/thing:6145160).  

**Hardware:**
- The project uses the Makerfabs-ESP32-S3-SPI-TFT-with-Touch board...
- A 320px240px image (.bmp) and a few icons for buttons...
- A text file with 15 lines of text (these are the key presses/macros), see example **sample_menu** text file...

**The Basics:**  
We start with a 480px by 320px background image, it consists of 16 button areas (identified by the red outlined squares).
Then using an image editor, edit Grid480x320.bmp file and import the desired button icons (64px by 64px images works well) into each of the outlined button icons positions. Once finished, export the final image as menu0.bmp and save to SD card.  

<br>
<img src="/images/blankbg.bmp" width=480>  
blankbg.bmp  
<br>

Next, create a text file called ***menu0*** and add 15 lines of text (each consisting of what we would need to type to execute the given programs, see **sample_menu** for example), then save the file to the SD card.  

```
01=freecad
02=[T]/usr/bin/flatpak run --branch=stable --arch=x86_64 --command=kicad --file-forwarding org.kicad.KiCad
03=/opt/sublime_text/sublime_text
04=/home/Mike/Downloads/arduino-1.8.19/arduino
05=[T]ls -l
06=[LA]3
07=/usr/share/playonlinux/playonlinux --run 'Fireworks'
08=firefox
09=xfce4-terminal
10=kcalc
11=[RA]4
12=meld
13=/home/Mike/AppImages/Stellarium.AppImage
14=[LC]c
15=[LC]v
```
**Special keys:**  
- [LA] = Left Alt - [RA] = Right Alt
- [LS] = Left Shift - [RS] = Right Shift
- [LC] = Left Ctrl - [RC] = Right Ctrl
- [SU] = Super Key
- [TA] = Tab (used in conjunction with Alt, Ctrl Keys)
- [F?] = Function Keys, F1 to F24 (example [F4], [F12] etc..) ***are these needed***)
- [T]   = Open A Terminal
- [T]file_name will the program_name inside a terminal (see menuitem02 above).
  
<br>
<img src="/images/menu0.bmp" width=360>  
Example of menu screen with a few icons added (/images/menu0.bmp)  
<br><br>

**Programming:**  
Install the Arduino IDE, add required libraries (see top of code.ino for required libraries), compile and upload *code.ini* sketch to the ESP32-S3 SPI board (don't forget to insert the SD card)...

**TBA:**  
Generating macro keyboard data is context/window-focus sensitive, that is, the active program will try to implement the keystrokes, so for testing under Linux I normally open a terminal manually, the press a key. I have added the [T] special key, to open a terminal before process the rest of the menu line (some programs require launching from a terminal, but it also helps to remove undesirable affects).  

**sample_menu:**  
Each line of the menu text can contain:  
-  A program name (can include the path) to launch, for example: ~/{HOME}/this_path/this_program.  
-  Special key combinations: [LA]+3 or [LC]+[F10] or [SHIFT]+[LC]+[LA]+x.
-  To open a terminal before processing a menu line use: [T]program_name_link_etc
-  Any combination of keys up to 128 characters.  

**After initial programming of the TFT board:**  
All changes are made to the *menu0* text file and/or the *menu.bmp* image on the SD card, no need to recompile as these changes are imported from SD card on every boot.  

*See **Grid480x320.bmp** example background image (without any icons) so you can add your own using image editing software.* 

**Future:**  
It should be possible to implement as many menus as required, for example, menus might allow loading of sub-menus, with each sub-menu having 13 buttons ( two would be required for navigation).
...

<br>
<img src="/images/Grid480x320.bmp" width=480>  
Optional 5x4 layout (20 key) (keysGrid480x320.bmp),  
Anything is possible with edits to the code, you could use a larger TFT too...  
<br><br>

This project is very much based on sample code edited to suit. My contribution was simply getting different code samples to work nicely together. I wrote a lot of C and Assembly code in another life, my skills are somewhat lacking especially in relation to Strings, please make allowances for any poor code (it's been a very long time)...

It appears to me that once you become familiar with an application, Macro Keyboards are really not required, at least, not for most basic programming tasks. But for the more complex Video/Audion editing, they still have a use. Even so, you can always repurpose this design for other applications.  

Hope it's useful to someone.  

***Some updates:***  
When dealing with multiple menus I did have an issues with stack being used up after many processes through the code, resulting in the device resetting. I have not determined the problem but if anyone has an ideas, please comment.
If (when) the issue with the stack has been resolved, this could be a very useful project...  

The ***tft_touch_keypad*** folder contains new code for multiple menus...  

The ***alt*** [folder](alt/currentTestCode.ino) containg current code used during my latest tests. I has removed unnecessary code. If anyone wants to help find Stack issue, check out this code...

Mike
