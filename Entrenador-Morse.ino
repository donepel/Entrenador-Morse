// defines associated specifically with Nokia 5110 LCD ScrnFuncts
#define PIN_SCE   7
#define PIN_RESET 6
#define PIN_DC    5
#define PIN_SDIN  4
#define PIN_SCLK  3

#define LCD_C     LOW
#define LCD_D     HIGH
#define LCD_X     84
#define LCD_Y     48

/*  ************************************************ Notes & Changes***********************************************
CHANGES:
20130515: Minor change moving functions between modules, created UtilFuncts.ino
          Implemented Verbose Pin#9
          Tightened setspeed() values and added console echo during verbose
20130512: Combined separate IF logic into more complex single statement in MagicMorse decode
20130510: Corrected maximum Elements to scan to correct Prosigns.  Added MaxElement constant
20130508: Implemented LCDCurrentLine() to identify the current LCD active line
          Implemented PARIS()
20130507: Implemented sub-screen to show WPM and allow changes before program starts main loop
          Implemented 10K potentiometer CT on pin A0 to control WPM from 5 to 40
          Moved Morse Key to Analog Pin A3 in prep for reading Analog signal
20130505: Clean-up & module restructioning & migrating character arrays to PROGMEM
          decode() changed to char MagicMorse to return char and checked in main loop
20130501: Major change: Speed Pin D9 is now inverted to create Normally Low behavior

NOTES:
MiniPRO Pins:
#__      Function_________________________________
A0       Center Tap of 10K pot to control WPM at bootup
A1       n/a
A2       n/a
A3       Morse Key (other side of Key to Gnd)
A4       n/a
A5       n/a
A6       n/a
A7       n/a
RESET    Reset
Tx  0    n/a (dedicated Serial Input)
Rx  1    Diag Output RS232-TTL 9600 BAUD Async used for diagnostics: VT100
PIN 2    n/a
PIN 3-7  Nokia Display  (specifics below)
PIN 8    Activate changes to NOKIA contrast B0 --> BF
PIN 9    Activate Verbose Diagnostics by pulling LOW
PIN 10   Force PARIS replay: tone & LCD, not diagnostic
PIN 11   Tone Out --> approx 750Hz @5V to Piezo
PIN 12   Green LED indicate DASH
PIN 13   Red   LED indicate DIT

Nokia 5110 Graphic LCD Pinout:
_______ Mini Pro____   _______ Nokia GLCD___      _____ test board cabling ___
#define PIN_SCE   7    LCD CE ....  Pin 2          Yellow
#define PIN_RESET 6    LCD RST .... Pin 1          Blue
#define PIN_DC    5    LCD Dat/Com. Pin 3  (DC)    Orange
#define PIN_SDIN  4    LCD SPIDat . Pin 4  (DIN)   White
#define PIN_SCLK  3    LCD SPIClk . Pin 5          Brown

//                     LCD Gnd .... Pin 2          Black
//                     LCD Vcc .... Pin 8          Red 3.3V
//                     LCD Vled ... Pin 7          Green (100 Ohms to Gnd)

*/

/*
  (c) Mickey R. Burnette, AKA: M. Ray Burne, author and software engineer: various blogs, Instructables, etc.
  Liquid Crystal library reference: http://arduino.cc/en/Reference/LiquidCrystal?from=Tutorial.LCDLibrary
  Timing semantics for Arduino published by Raron:
      http://raronoff.wordpress.com/2010/12/16/morse-endecoder/
  Magic Morse Algorithm Copyright (c) 2011, 2012, 2013 by mrburnette and published at:
      http://code.google.com/p/morse-endecoder/wiki/Usage  
      http://www.instructables.com/id/Morse-Code-Magic-An-Algorithm/
      http://www.youtube.com/watch?v=9kZOqdeUl2w&feature=youtube_gdata
      http://www.picaxeforum.co.uk/showthread.php?19088-Morse-Code-Decoder-for-Cheap-using-a-20X2&p=177912#post177912  
  Contact Ray Burne: magic.morse @ gmail.com - All commercial rights reserved worldwide
  >>> Publically published articles with source grants end-use one (1) personal use license ONLY <<<

Target compiles:  ATmega328P - profile Mini w/328P @ 16MHz 
    â˜º 20130525 - Version 8a  Implemented Auto-setup for WPM and storing in EEPROM
    â˜º 20130525 - Version 8   Merged Nokia base routines with 2x16 LCD
    â˜º 20130523 - Version 6   Configured LCD on Bareboard to be same pinout as on MiniPro, same codebase
    â˜º 20130421 - Version 4.  Board-duio version with LCD and variable resistor for speed timing
    â˜º 20121001 - Version .20 rewrite to eliminate object oriented code for faster inline
    â˜º 20120922 - Version .10 with WPM changes based on D0
    â˜º 20120919 - Edited MM[] + code to allow some prosigns as in PICAXE code
    â˜º          - Works from 25WPM to 40WPM using ARRL test files (w/ WPM = 30)
    â˜º 20120918 - Magic Morse Interface MM[] array implemented ClipperDigital
    â˜º          - Works from  5WPM to 35WPM using ARRL test files (w/ WPM = 20)
    â˜º 20120917 - Initial code structual build from raron timing strutures

  // Interfacing requirements for use of digital I/O (NPN audio clipper circuit)
     2.2K / 0.1uF (0.2) and input resistance 110 Ohms to 50 Ohms
  // Compile options: (commercial = UNO)

                            MiniPro backpack        328P Board-duino
                           lcd(7, 6, 2, 3, 4, 5)       <ditto>
   * LCD RS     pin to     digital pin 7                  "
   * LCD Enable pin to     digital pin 6                  "
   * LCD D7     pin to     digital pin 5                  "
   * LCD D6     pin to     digital pin 4                  "
   * LCD D5     pin to     digital pin 3                  "
   * LCD D4     pin to     digital pin 2                  "
   * LCD R/W pin to ground
   * 10K variable resistor (contrast):
       * ends to +5V and ground
       * wiper to LCD VO pin (pin 3)


        elements	WPM	tElements	mS / element
        50	          5	  250	          240
        50	         10	  500	          120
        50	         15	  750	           80
        50	         20	 1000	           60
        50	         25	 1250	           48
        50	         30	 1500	           40
        50	         35	 1750	           34
        50	         40	 2000	           30
        50	         45	 2250	           27
        50	         50	 2500	           24


                                      +-\/-+
Reset                           PC6  1|    |28  PC5 (AI 5)
Rx                        (D 0) PD0  2|    |27  PC4 (AI 4)
Tx                        (D 1) PD1  3|    |26  PC3 (AI 3)             Contact (non-Gnd) end of Morse Key
LCD D4                    (D 2) PD2  4|    |25  PC2 (AI 2)
LCD D5               PWM+ (D 3) PD3  5|    |24  PC1 (AI 1)
LCD D6                    (D 4) PD4  6|    |23  PC0 (AI 0)
+5                              VCC  7|    |22  GND
GND                             GND  8|    |21  AREF
XTL1                            PB6  9|    |20  AVCC
XTL2                            PB7 10|    |19  PB5 (D 13)             Green LED  - DAH
LCD D7               PWM+ (D 5) PD5 11|    |18  PB4 (D 12)             Red LED    - DIT
LCD E                PWM+ (D 6) PD6 12|    |17  PB3 (D 11) PWM         (PWM 750Hz) to Piezo 
LCD RS                    (D 7) PD7 13|    |16  PB2 (D 10) PWM         Force PARIS (momentary Gnd)
                          (D 8) PB0 14|    |15  PB1 (D  9) PWM         Verbose console option - Debug
                                      +----+


*/
