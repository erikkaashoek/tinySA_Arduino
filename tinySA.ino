/* tinySA.ino - Si4432 Spectrum analyzer
 *
 * Copyright (c) 2017 Erik Kaashoek <erik@kaashoek.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#if defined(ARDUINO_ARCH_SAMD) 
#define  Serial SerialUSB
#endif

// Comment out below line if you do not want a local user interface
#define USE_ROTARY  1
#define USE_DISPLAY 1
// #define USE_SSD1306 1
#define USE_ILI9341 1
// #define USE_SI4463  1

#ifdef USE_SI4463
#include "./Si446x.h" 
#endif


//------------------------------------------ Display ------------------------------------
#ifdef USE_SSD1306
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//#define SCREEN_WIDTH 128 // OLED display width, in pixels
//#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DISPLAY_POINTS 100
#endif



#ifdef USE_ILI9341
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_DC A5
#define TFT_CS A4

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define DISPLAY_POINTS 300

#endif

//#define SCREEN_WIDTH 320 // OLED display width, in pixels
//#define SCREEN_HEIGHT 240 // OLED display height, in pixels

// The onboard led is blinked during serial transfer
#define tinySA_led 13

#ifndef DISPLAY_POINTS
#define DISPLAY_POINTS 100
#endif

//--------------------- Generic display ---------------

void clearDisplay()
{
#ifdef USE_SSD1306  
  tft.clearDisplay();
#endif
#ifdef USE_ILI9341
  tft.fillScreen(ILI9341_BLACK);
#endif
}

void textWhite()
{
#ifdef USE_SSD1306  
   tft.setTextSize(1);
   tft.setTextColor(SSD1306_WHITE);        // Draw white text
#define DISPLAY_WHITE SSD1306_WHITE
#define DISPLAY_BLACK SSD1306_BLACK
#define DISPLAY_GRAY SSD1306_WHITE
#define DISPLAY_YELLOW SSD1306_WHITE
#define DISPLAY_INVERSE SSD1306_INVERSE
#endif
#ifdef USE_ILI9341
  tft.setTextColor(ILI9341_WHITE); 
#define DISPLAY_WHITE ILI9341_WHITE
#define DISPLAY_BLACK ILI9341_BLACK
#define DISPLAY_GRAY ILI9341_WHITE
#define DISPLAY_YELLOW ILI9341_YELLOW
#define DISPLAY_INVERSE  ILI9341_WHITE
#define SCREEN_WIDTH  tft.width()
#endif
}

void sendDisplay()
{
#ifdef USE_SSD1306  
   tft.display();
#endif
#ifdef USE_ILI9341
    yield();
#endif
}

//---------------- Drawing ------------------

#ifdef USE_ILI9341

const int dX = 30 ;
const int dY = 24 ;
const int oX = 20 ;
const int oY = 24 ;
const int DOTS = 10 * dX - 2 ;  // FOR ADAFRUIT RA8875

 
void DrawCheckerBoard() 
{
  // VERTICAL
  for (int y=0; y<9; y++)
  {
    // HORIZONTAL
    for (int x=0; x<10; x++)
    {
      tft.drawRect(oX + x*dX, y*dY+oY, dX, dY, ILI9341_DARKGREY);
      tft.fillRect(oX + x*dX+1, y*dY+1+oY, dX-2, dY-2, DISPLAY_BLACK);
    }
    tft.drawRect(oX, oY, 10 * dX, 9 * dY, ILI9341_DARKGREY);  
  }
  tft.drawRect(oX, oY, 10 * dX, 9 * dY, ILI9341_DARKGREY);  
}
 
 
void DrawRectangleMenue() 
{
  // VERTICAL
  for (int y=0; y<9; y++)
  {
    // HORIZONTAL
    for (int x=0; x<10; x++)
    {
      tft.drawRect(oX, oY, 10 * dX, 9 * dY, DISPLAY_WHITE);
      tft.fillRect(oX+1, oY+1, 10 * dX - 2, 9 * dY - 2, DISPLAY_BLACK);
    }
  }  
}
 
/* 
void ShowVerticalScale() 
{
  // CLEAR TEXTAREA
  tft.fillRect(0, 0, oX-2, 420, DISPLAY_BLACK);
  // NOW WRITE NEW VALUES
//  tft.textMode();
  tft.textEnlarge(0);
  char string[6]; 
  int level ;
  int Top ;
  if (AttSet == 0) Top = (int)( ThruLoss + MaxIFLevel );
  if (AttSet == 1) Top = (int)( ThruLoss + MaxIFLevel + AttLoss );
 
  for (int y=0; y<=9; y++)
  {
      tft.textSetCursor(0, y * dY);
      tft.textTransparent(DISPLAY_WHITE);
      level = Top - y * 10 ;
      itoa(level,string,10);
      if (level == 0)
      {
      for (int z=6; z>0; z--) string[z] = string[z-1] ; 
      string[0] = 32 ; // space
      }
      if (level >= 0)
      {
      for (int z=6; z>0; z--) string[z] = string[z-1] ; 
      }
      if (level > 0) string[0] = 43 ; // plus
      if (abs(level) < 100)
      {
      for (int z=6; z>0; z--) string[z] = string[z-1] ;
      string[0] = 32 ; // space 
      }
        
      tft.textWrite(string);
  }  
}

 
void DrawLine() 
{
  long Y1, Y2 ;
  if (AttSet == 0) TopLevel = MaxIFLevel + ThruLoss  ;
  if (AttSet == 1) TopLevel = MaxIFLevel + AttLoss + ThruLoss ;
  // HORIZONTAL
  for (int x=0; x < DOTS-2 ; x+=1)
  {
  if (Sample[x] >= TopLevel) Sample[x] = TopLevel - 0.5 ;
  if (Sample[x+1] >= TopLevel) Sample[x+1] = TopLevel - 0.5 ;
  if (Sample[x] <= (TopLevel - 90.0)) Sample[x] = TopLevel - 89.5 ;
  if (Sample[x+1] <= (TopLevel - 90.0)) Sample[x+1] = TopLevel - 89.5 ;
  Y1 = (int)(dY * abs((TopLevel - Sample[x])   * 0.1 ));
  Y2 = (int)(dY * abs((TopLevel - Sample[x+1]) * 0.1 ));
  tft.drawLine(x+oX+2, Y1+oY, x+oX+3, Y2+oY, DISPLAY_YELLOW);
  tft.drawLine(x+oX+2, Y1+oY+1, x+oX+3, Y2+oY+1, DISPLAY_YELLOW);
  }
}
 
*/ 

#endif

// -------------------- Rotary -------------------------------------------
#ifdef USE_ROTARY


// Encode input pins, these should be interrupt enabled inputs
int R_pinEncA=7;
int R_pinEncB=6;
 
volatile int R_counter = 0; 
static int R_counter_oud = 0;        
static byte R_abOld = 0;      

void R_setup() {
  pinMode(R_pinEncA, INPUT_PULLUP);
  pinMode(R_pinEncB, INPUT_PULLUP);

  attachInterrupt(R_pinEncA, R_pinAction, CHANGE);
  attachInterrupt(R_pinEncB, R_pinAction, CHANGE);
}


int R_delta() {
    int c = R_counter - R_counter_oud;
    R_counter_oud = R_counter;
    static int old_pins = 0;
    int new_pins =  (digitalRead(R_pinEncB) << 1) | digitalRead(R_pinEncA);
    if (new_pins != old_pins) {
      old_pins = new_pins;
/*

   Serial.print ("Rotary = ");
   Serial.print (digitalRead(R_pinEncA));
   Serial.print (":");
   Serial.print (digitalRead(R_pinEncB));
   Serial.print (" = ");
   Serial.println (c);
   Serial.print ("  Rotary2 = ");
   Serial.print (digitalRead(R_pinEncA));
   Serial.print (":");
   Serial.println (digitalRead(R_pinEncB));
*/
  }    
    return (c);
}

int R_count() {
    R_counter_oud = R_counter;
    return (R_counter);
}

#define DIR_CCW 0x10
#define DIR_CW 0x20


#define R_START 0x0

#define ENABLE_HALF_STEP
#ifdef ENABLE_HALF_STEP
// Use the half-step state table (emits a code at 00 and 11)
#define R_CCW_BEGIN   0x1
#define R_CW_BEGIN    0x2
#define R_START_M     0x3
#define R_CW_BEGIN_M  0x4
#define R_CCW_BEGIN_M 0x5

const unsigned char ttable[][4] = 
{
  // 00                  01              10            11
  {R_START_M,           R_CW_BEGIN,     R_CCW_BEGIN,  R_START},           // R_START (00)
  {R_START_M | DIR_CCW, R_START,        R_CCW_BEGIN,  R_START},           // R_CCW_BEGIN
  {R_START_M | DIR_CW,  R_CW_BEGIN,     R_START,      R_START},           // R_CW_BEGIN
  {R_START_M,           R_CCW_BEGIN_M,  R_CW_BEGIN_M, R_START},           // R_START_M (11)
  {R_START_M,           R_START_M,      R_CW_BEGIN_M, R_START | DIR_CW},  // R_CW_BEGIN_M 
  {R_START_M,           R_CCW_BEGIN_M,  R_START_M,    R_START | DIR_CCW}  // R_CCW_BEGIN_M
};
#else
// Use the full-step state table (emits a code at 00 only)
#define R_CW_FINAL   0x1
#define R_CW_BEGIN   0x2
#define R_CW_NEXT    0x3
#define R_CCW_BEGIN  0x4
#define R_CCW_FINAL  0x5
#define R_CCW_NEXT   0x6

const unsigned char ttable[7][4] = 
{
  // 00         01           10           11
  {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},           // R_START
  {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},  // R_CW_FINAL
  {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},           // R_CW_BEGIN
  {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},           // R_CW_NEXT
  {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},           // R_CCW_BEGIN
  {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW}, // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START}            // R_CCW_NEXT
};
#endif 





volatile unsigned char R_state = 0;


void R_pinAction() {
  unsigned char pinstate = (digitalRead(R_pinEncB) << 1) | digitalRead(R_pinEncA);
  unsigned char pinstate_t = (digitalRead(R_pinEncB) << 1) | digitalRead(R_pinEncA);
  while (pinstate != pinstate_t) {
    pinstate = pinstate_t;
    pinstate_t = (digitalRead(R_pinEncB) << 1) | digitalRead(R_pinEncA);
  }
  R_state = ttable[R_state & 0xf][pinstate];  
//  Serial.print("pinstate = ");
//  Serial.print(pinstate);
//  Serial.print(" R_state = ");
//  Serial.println(R_state);
  if (R_state & 0x30) {
    if ((R_state & 0x30) == DIR_CCW)
      R_counter++;
    else
      R_counter--;
//    Serial.print("r_counter = ");
//    Serial.println(R_counter);
  }
  return;
}

#endif

///-------------------------------------------------------- SI4432 start ----------------------------------------------

// PINS SI4432, you can change these to any pin you want

const int SI_nSEL[4] = { 0,5, 11, 12 }; // #4 is dummy!!!!!!
const int SI_SCLK = 1 ;
const int SI_SDI = 2 ;
const int SI_SDO = 3 ;

// Use this to tune the xtal oscilators on the SI4432 modules to the exact frquency
#define V0_XTAL_CAPACITANCE 0x64
#define V1_XTAL_CAPACITANCE 0x64


byte SI4432REG[129] ;
float bandwidth = 34.6 ;

// currently selectd SI4432
int SI4432_Sel = 0;

void SI4432_Write_Byte(byte ADR, byte DATA )
{
  ADR |= 0x80 ; // RW = 1
  digitalWrite(SI_SCLK, LOW);
  digitalWrite(SI_nSEL[SI4432_Sel], LOW);
  shiftOut(SI_SDI , SI_SCLK , MSBFIRST , ADR );
  shiftOut(SI_SDI , SI_SCLK , MSBFIRST , DATA );
  digitalWrite(SI_nSEL[SI4432_Sel], HIGH);
}

byte SI4432_Read_Byte( byte ADR )
{
  byte DATA ;
  digitalWrite(SI_SCLK, LOW);
  digitalWrite(SI_nSEL[SI4432_Sel], LOW);
  shiftOut(SI_SDI , SI_SCLK , MSBFIRST , ADR );
  DATA = shiftIn(SI_SDO , SI_SCLK , MSBFIRST );
  digitalWrite(SI_nSEL[SI4432_Sel], HIGH);
  return DATA ;
}

void SI4432_Reset()
{
  int count = 0;
  // always perform a system reset (don't send 0x87) 
  SI4432_Write_Byte( 0x07, 0x80);
  delay(10);
  // wait for chiprdy bit
  while (count++ < 100 && ( SI4432_Read_Byte ( 0x04 ) & 0x02 ) == 0) { 
    delay(10);
    Serial.print("Waiting for SI4432 ");
    Serial.println(SI4432_Sel);
  }
}

#if 0
float SI4432_SET_RBW(float WISH)
{
procedure SI4432_Set_BW_FSK ( dword in BW_Hz ) is
  var byte IF_filset [] = {
    1,2,3,4,5,6,7,1,2,3,
    4,5,6,7,1,2,3,4,5,6,
    7,1,2,3,4,5,6,7,1,2,
    3,4,5,6,7,1,2,3,4,5,
    6,7,4,5,9,15,1,2,3,4,
    8,9,10,11,12,13,14 }

  -- set the largest bandwidth (used if no valid value is found)
  var byte Index = count ( IF_Bandwidth ) - 1 
  
  -- loop until a bandwidth larger or equal to the desired bandwidth is found
  for count ( IF_Bandwidth ) using i loop
    if IF_Bandwidth [i] >= WISH then
      -- if found, remember the index and leave the loop
      Index = i
      exit loop
    end if
  end loop
 
  -- get the parts from the different lookup tables
  ndec_exp    = IF_ndec_exp    [ Index ]
  dwn3_bypass = IF_dwn3_bypass [ Index ]
  filset      = IF_filset      [ Index ]  

  -- merge the parts and write them to the bandwidth register
  var byte Value = (dwn3_bypass << 7) | (ndec_exp << 4) | filset  
  SI4432_Write ( 0x1C, Value )
}   
#else
float SI4432_SET_RBW(float WISH)
{
  byte ndec = 5 ;
  byte dwn3 = 0 ;
  byte fils = 1 ;
  float rxosr = 12.5 ;
  float REAL = 2.6 ;   // AS CLOSE AS POSSIBLE TO "WISH" :-)
  // YES, WE KNOW THIS IS SLOW
#if 0 // Too many resolutions, not needed
  if (WISH > 2.6) {
    ndec = 5 ;
    fils = 2 ;
    REAL = 2.8 ;
  }
#endif
  if (WISH > 2.8) {
    ndec = 5 ;
    fils = 3 ;
    REAL = 3.1 ;
  }
#if 0 // Too many resolutions, not needed
  if (WISH > 3.1) {
    ndec = 5 ;
    fils = 4 ;
    REAL = 3.2 ;
  }
  if (WISH > 3.2) {
    ndec = 5 ;
    fils = 5 ;
    REAL = 3.7 ;
  }
  if (WISH > 3.7) {
    ndec = 5 ;
    fils = 6 ;
    REAL = 4.2 ;
  }
  if (WISH > 4.2) {
    ndec = 5 ;
    fils = 7 ;
    REAL = 4.5 ;
  }
  if (WISH > 4.5) {
    ndec = 4 ;
    fils = 1 ;
    REAL = 4.9 ;
  }
  if (WISH > 4.9) {
    ndec = 4 ;
    fils = 2 ;
    REAL = 5.4 ;
  }
  if (WISH > 5.4) {
    ndec = 4 ;
    fils = 3 ;
    REAL = 5.9 ;
  }
  if (WISH > 5.9) {
    ndec = 4 ;
    fils = 4 ;
    REAL = 6.1 ;
  }
  if (WISH > 6.1) {
    ndec = 4 ;
    fils = 5 ;
    REAL = 7.2 ;
  }
  if (WISH > 7.2) {
    ndec = 4 ;
    fils = 6 ;
    REAL = 8.2 ;
  }
  if (WISH > 8.2) {
    ndec = 4 ;
    fils = 7 ;
    REAL = 8.8 ;
  }
  if (WISH > 8.8) {
    ndec = 3 ;
    fils = 1 ;
    REAL = 9.5 ;
  }
#endif
  if (WISH > 9.5) {
    ndec = 3 ;
    fils = 2 ;
    REAL = 10.6 ;
  }
#if 0 // Too many resolutions, not needed
  if (WISH > 10.6) {
    ndec = 3 ;
    fils = 3 ;
    REAL = 11.5 ;
  }
  if (WISH > 11.5) {
    ndec = 3 ;
    fils = 4 ;
    REAL = 12.1 ;
  }
  if (WISH > 12.1) {
    ndec = 3 ;
    fils = 5 ;
    REAL = 14.2 ;
  }
  if (WISH > 14.2) {
    ndec = 3 ;
    fils = 6 ;
    REAL = 16.2 ;
  }
  if (WISH > 16.2) {
    ndec = 3 ;
    fils = 7 ;
    REAL = 17.5 ;
  }
  if (WISH > 17.5) {
    ndec = 2 ;
    fils = 1 ;
    REAL = 18.9 ;
  }
  if (WISH > 18.9) {
    ndec = 2 ;
    fils = 2 ;
    REAL = 21.0 ;
  }
  if (WISH > 21.0) {
    ndec = 2 ;
    fils = 3 ;
    REAL = 22.7 ;
  }
  if (WISH > 22.7) {
    ndec = 2 ;
    fils = 4 ;
    REAL = 24.0 ;
  }
  if (WISH > 24.0) {
    ndec = 2 ;
    fils = 5 ;
    REAL = 28.2 ;
  }
#endif
  if (WISH > 28.2) {
    ndec = 2 ;
    fils = 6 ;
    REAL = 32.2 ;
  }
#if 0 // Too many resolutions, not needed
  if (WISH > 32.2) {
    ndec = 2 ;
    fils = 7 ;
    REAL = 34.7 ;
  }
  if (WISH > 34.7) {
    ndec = 1 ;
    fils = 1 ;
    REAL = 37.7 ;
  }
  if (WISH > 37.7) {
    ndec = 1 ;
    fils = 2 ;
    REAL = 41.7 ;
  }
  if (WISH > 41.7) {
    ndec = 1 ;
    fils = 3 ;
    REAL = 45.2 ;
  }
  if (WISH > 45.2) {
    ndec = 1 ;
    fils = 4 ;
    REAL = 47.9 ;
  }
  if (WISH > 47.9) {
    ndec = 1 ;
    fils = 5 ;
    REAL = 56.2 ;
  }
  if (WISH > 56.2) {
    ndec = 1 ;
    fils = 6 ;
    REAL = 64.1 ;
  }
  if (WISH > 64.1) {
    ndec = 1 ;
    fils = 7 ;
    REAL = 69.2 ;
  }
  if (WISH > 69.2) {
    ndec = 0 ;
    fils = 1 ;
    REAL = 75.2 ;
  }
  if (WISH > 75.2) {
    ndec = 0 ;
    fils = 2 ;
    REAL = 83.2 ;
  }
  if (WISH > 83.2) {
    ndec = 0 ;
    fils = 3 ;
    REAL = 90.0 ;
  }
  if (WISH > 90.0) {
    ndec = 0 ;
    fils = 4 ;
    REAL = 95.3 ;
  }
#endif
  if (WISH > 95.3) {
    ndec = 0 ;
    fils = 5 ;
    REAL = 112.1 ;
  }
#if 0 // Too many resolutions, not needed
  if (WISH > 112.1) {
    ndec = 0 ;
    fils = 6 ;
    REAL = 127.9 ;
  }
  if (WISH > 127.9) {
    ndec = 0 ;
    fils = 7 ;
    REAL = 137.9 ;
  }
  if (WISH > 137.9) {
    ndec = 1 ;
    fils = 4 ;
    REAL = 142.8 ;
  }
  if (WISH > 142.8) {
    ndec = 1 ;
    fils = 5 ;
    REAL = 167.8 ;
  }
  if (WISH > 167.8) {
    ndec = 1 ;
    fils = 9 ;
    REAL = 181.1 ;
  }
  if (WISH > 181.1) {
    ndec = 0 ;
    fils = 15 ;
    REAL = 191.5 ;
  }
  if (WISH > 191.5) {
    ndec = 0 ;
    fils = 1 ;
    REAL = 225.1 ;
  }
  if (WISH > 225.1) {
    ndec = 0 ;
    fils = 2 ;
    REAL = 248.8 ;
  }
  if (WISH > 248.8) {
    ndec = 0 ;
    fils = 3 ;
    REAL = 269.3 ;
  }
  if (WISH > 269.3) {
    ndec = 0 ;
    fils = 4 ;
    REAL = 284.9 ;
  }
#endif
  if (WISH > 284.9) {
    ndec = 0 ;
    fils = 8 ;
    REAL = 335.5 ;
  }
#if 0 // Too many resolutions, not needed
  if (WISH > 335.5) {
    ndec = 0 ;
    fils = 9 ;
    REAL = 361.8 ;
  }
  if (WISH > 361.8) {
    ndec = 0 ;
    fils = 10 ;
    REAL = 420.2 ;
  }
  if (WISH > 420.2) {
    ndec = 0 ;
    fils = 11 ;
    REAL = 468.4 ;
  }
  if (WISH > 468.4) {
    ndec = 0 ;
    fils = 12 ;
    REAL = 518.8 ;
  }
  if (WISH > 518.8) {
    ndec = 0 ;
    fils = 13 ;
    REAL = 577.0 ;
  }
#endif
  if (WISH > 577.0) {
    ndec = 0 ;
    fils = 14 ;
    REAL = 620.7 ;
  }

  if (WISH > 137.9) dwn3 = 1 ;

  byte BW = (dwn3 << 7) | (ndec << 4) | fils ;

  SI4432_Write_Byte(0x1C , BW ) ;

  rxosr = 500.0 * ( 1.0 + 2.0 * dwn3 ) / ( pow(2.0, (ndec-3.0)) * REAL );

  byte integer = (int)rxosr ;
  byte fractio = (int)((rxosr - integer) * 8 ) ;
  byte memory = (integer << 3) | (0x07 & fractio) ;

  SI4432_Write_Byte(0x20 , memory ) ;
  return REAL ;
}

#endif

void SI4432_Set_Frequency ( long Freq ) {
  int hbsel;
  long Carrier;
  if (Freq >= 480000000) {
    hbsel = 1;
    Freq = Freq / 2;
  } else {
    hbsel = 0;
  }  
  int sbsel = 1;
  int N = Freq / 10000000;
  Carrier = ( 4 * ( Freq - N * 10000000 )) / 625;
  int Freq_Band = ( N - 24 ) | ( hbsel << 5 ) | ( sbsel << 6 );
  SI4432_Write_Byte ( 0x75, Freq_Band );
  SI4432_Write_Byte ( 0x76, (Carrier>>8) & 0xFF );
  SI4432_Write_Byte ( 0x77, Carrier & 0xFF  );
  delay(2);
}  

int SI4432_RSSI()
{
  int RSSI_RAW;
  // SEE DATASHEET PAGE 61
#ifdef USE_SI4463
  if (SI4432_Sel == 2) {
    RSSI_RAW = Si446x_getRSSI();
  } else
#endif
    RSSI_RAW = (unsigned char)SI4432_Read_Byte( 0x26 ) ;
//  float dBm = 0.5 * RSSI_RAW - 120.0 ;
  // Serial.println(dBm,2);
  return RSSI_RAW ;
}


void SI4432_Sub_Init()
{
  SI4432_Reset();

#if 0

  SI4432_Write_Byte(0x75, 0x53);
  SI4432_Write_Byte(0x76, 0x62);
  SI4432_Write_Byte(0x77, 0x00);

  SI4432_Write_Byte(0x6E, 0x19);
  SI4432_Write_Byte(0x6F, 0x9A);
  SI4432_Write_Byte(0x70, 0x04);
  SI4432_Write_Byte(0x58, 0xC0);

  SI4432_Write_Byte(0x1C, 0x81);
  SI4432_Write_Byte(0x20, 0x78);
  SI4432_Write_Byte(0x21, 0x01);
  SI4432_Write_Byte(0x22, 0x11);
  SI4432_Write_Byte(0x23, 0x11);
  SI4432_Write_Byte(0x24, 0x01);
  SI4432_Write_Byte(0x25, 0x13);
  SI4432_Write_Byte(0x2C, 0x28);
  SI4432_Write_Byte(0x2D, 0x0C);
  SI4432_Write_Byte(0x2E, 0x28);
  SI4432_Write_Byte(0x1F, 0x03);
  SI4432_Write_Byte(0x69, 0x60);

  // disable all interrupts
  SI4432_Write_Byte ( 0x06, 0x00 );

  // Set the sytem in Ready mode. PLL on, RX manual receive
  SI4432_Write_Byte ( 0x07, 0x07 );
   
#else  
  // Enable receiver chain
//  SI4432_Write_Byte(0x07, 0x05);
  // Clock Recovery Gearshift Value
  SI4432_Write_Byte(0x1F, 0x00);
  // IF Filter Bandwidth
  bandwidth = SI4432_SET_RBW(bandwidth) ;
  // REG 0x20 is updated with the IF Filter bandwidth
  // Register 0x75 Frequency Band Select
  byte sbsel = 1 ;  // recommended setting
  byte hbsel = 0 ;  // low bands
  byte fb = 19 ;    // 430–439.9 MHz
  byte FBS = (sbsel << 6 ) | (hbsel << 5 ) | fb ;
//  SI4432_Write_Byte(0x75, FBS) ;
  SI4432_Write_Byte(0x75, 0x46) ;
  // Register 0x76 Nominal Carrier Frequency
  // WE USE 433.92 MHz
  // Si443x-Register-Settings_RevB1.xls
//  SI4432_Write_Byte(0x76, 0x62) ;
  SI4432_Write_Byte(0x76, 0x00) ;
  // Register 0x77 Nominal Carrier Frequency
  SI4432_Write_Byte(0x77, 0x00) ;
  // RX MODEM SETTINGS
  SI4432_Write_Byte(0x1C, 0x81) ;
  SI4432_Write_Byte(0x1D, 0x3C) ;
  SI4432_Write_Byte(0x1E, 0x02) ;
  SI4432_Write_Byte(0x1F, 0x03) ;
  // SI4432_Write_Byte(0x20, 0x78) ;
  SI4432_Write_Byte(0x21, 0x01) ;
  SI4432_Write_Byte(0x22, 0x11) ;
  SI4432_Write_Byte(0x23, 0x11) ;
  SI4432_Write_Byte(0x24, 0x01) ;
  SI4432_Write_Byte(0x25, 0x13) ;
  SI4432_Write_Byte(0x2A, 0xFF) ;
  SI4432_Write_Byte(0x2C, 0x28) ;
  SI4432_Write_Byte(0x2D, 0x0C) ;
  SI4432_Write_Byte(0x2E, 0x28) ;


  SI4432_Write_Byte(0x69, 0x10); // No AGC, max LNA of 20dB
  SI4432_Write_Byte(0x69, 0x60); // No AGC, max LNA of 20dB

#endif

// GPIO automatic antenna switching
  SI4432_Write_Byte(0x0B, 0x12) ;
  SI4432_Write_Byte(0x0C, 0x15) ;
}

void SI4432_Init()
{
  pinMode(SI_nSEL[0], OUTPUT);
  pinMode(SI_nSEL[1], OUTPUT);
  pinMode(SI_SCLK, OUTPUT);
  pinMode(SI_SDI,  OUTPUT);
  pinMode(SI_SDO,  INPUT_PULLUP);

  digitalWrite(SI_SCLK, LOW);
  digitalWrite(SI_SDI, LOW);

  digitalWrite(SI_nSEL[0], HIGH);
  digitalWrite(SI_nSEL[1], HIGH);

  SI4432_Reset();
//DebugLine("IO set");
  SI4432_Sel = 0;
  SI4432_Sub_Init();

  SI4432_Sel = 1;
  SI4432_Sub_Init();
//DebugLine("1 init done");


  SI4432_Sel = 0;
  SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain
  SI4432_Write_Byte(0x09, V0_XTAL_CAPACITANCE);// Tune the crystal
  SI4432_Set_Frequency(433920000);
  SI4432_Write_Byte(0x0D, 0x1F) ; // Set GPIO2 output to ground


  SI4432_Sel = 1;
  SI4432_Write_Byte(0x7, 0x0B); // start TX
  SI4432_Write_Byte(0x09, V1_XTAL_CAPACITANCE);// Tune the crystal
  SI4432_Set_Frequency(443920000);
  SI4432_Write_Byte(0x6D, 0x1F);//Set full power
  
  SI4432_Write_Byte(0x0D, 0xC0) ; // Set GPIO2 maximumdrive and clock output
  SI4432_Write_Byte(0x0A, 0x02) ; // Set 10MHz output
}

void SetPowerReference(int freq)
{
  SI4432_Sel = 1;         //Select Lo module
  if (freq < 0 || freq > 7 ) {
    SI4432_Write_Byte(0x0D, 0x1F) ; // Set GPIO2 to GND
  } else {
    SI4432_Write_Byte(0x0D, 0xC0) ; // Set GPIO2 maximumdrive and clock output
    SI4432_Write_Byte(0x0A, freq & 0x07) ; // Set GPIO2 frequency
  }
}

//------------PE4302 -----------------------------------------------

// Comment out this define to use parallel mode PE4302
#define PE4302_serial

#ifdef PE4302_serial
// Clock and data pints are shared with SI4432
// Serial mode LE pin
#define PE4302_en 10
#else
//Parallel mode bit 0 pin number, according below line the PE4302 is connected to lines A0,A1,A2,A3,A4,A5
#define PE4302_pinbase A0
#endif

void PE4302_init() {
#ifdef PE4302_serial
  pinMode(PE4302_en, OUTPUT);
  digitalWrite(PE4302_en, LOW);
#else
  for (int i=0; i<6; i++) pinMode(i+PE4302_pinbase, OUTPUT);          // Setup attenuator at D6 - D11
#endif
}

void PE4302_Write_Byte(byte DATA )
{
#ifdef PE4302_serial
//Serial mode output  
  digitalWrite(SI_SCLK, LOW);
  shiftOut(SI_SDI , SI_SCLK , MSBFIRST , DATA );
  digitalWrite(PE4302_en, HIGH);
  digitalWrite(PE4302_en, LOW);
#else
// Parallel mode output
  for (int i=0; i<6;i++) {
    digitalWrite(i+PE4302_pinbase, p & (1<<i));
  }
#endif
}


// ----------------------- rotary -----------------------------------

const int buttonPin = 8;    // the number of the pushbutton pin
const int backButtonPin = 9;
enum buttont_event {shortClickRelease=1, longClick=2, longClickRelease=3, shortBackClickRelease=4, longBackClick=5, longBackClickRelease=6, buttonRotateUp=7, buttonRotateDown=8 };
enum button_state {buttonUp, buttonDown, buttonLongDown};
int buttonState = buttonUp;             // the current reading from the input pin
int backButtonState = buttonUp;             // the current reading from the input pin
int buttonEvent = 0;
int lastButtonRead = HIGH;   // the previous reading from the input pin
int lastBackButtonRead = HIGH;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long longPressDelay = 350;    // the longpress time; increase if the output flickers

long incr;
long incrBase = 1000000;
int incrBaseDigit = 7 ; 

// ---------------------------------------------------

#define MAX_VFO 3
long lFreq[MAX_VFO] = { 0,100000000,433700000};
int dataIndex = 0;


void showFreq(unsigned long f)
{
  char t[16];
  int digit = 10;
  int leading = 1;
  unsigned long divider = 1000000000;
  int i=3;
  t[0] = '0' + dataIndex;
  t[1] = ':';
  t[2] = ' ';
  while (digit>0)
  {
    if (digit == 6) {
      t[i++] = '.';
      leading = 0;
    }
    if (digit == incrBaseDigit)
      t[i++] = '[';
    if (f / divider > 0)
      leading = 0;
    t[i++] = ((int) (f / divider)) + '0';
    f %= divider;      
    if (digit == incrBaseDigit)
      t[i++] = ']';
    divider /= 10;
    digit -= 1;    
  }
  t[i++] = 0;
//  Serial.println(t);
#ifdef USE_DISPLAY
  tft.fillRect(0, 0, tft.width(), oY-2, DISPLAY_BLACK);
//  clearDisplay();
  textWhite();        // Draw white text
  tft.setCursor(0, 1);
  tft.print(t);
  sendDisplay();
#endif
}

void ChangeFrequency(long v[])
{
static long old_time;
  if (buttonState == buttonLongDown) 
  {
    if (buttonEvent == buttonRotateUp) {
      if (incrBase > 10) {
        incrBase /= 10;
        incrBaseDigit--;
      }
    } else if ( buttonEvent == buttonRotateDown){
      if (incrBase < 100000000) {
        incrBase *= 10;
        incrBaseDigit++;
      }
    }
    showFreq(v[dataIndex]); // to update selected digit
    Serial.print(incrBase);
    Serial.println(F(": incrbase set"));
  }
  else if ( buttonEvent == buttonRotateDown ||  buttonEvent == buttonRotateUp){
 //   incr = 1;
    v[dataIndex] = (v[dataIndex] / incrBase) * incrBase; // round to incrBase
    if (buttonEvent == buttonRotateUp) {
      v[dataIndex] += incr*incrBase;
      if (v[dataIndex] > 440000000)
        v[dataIndex] = 440000000;
    } else {
      v[dataIndex] -= incr*incrBase;
      if (v[dataIndex] < 0)
        v[dataIndex] = 0;
    }
    showFreq(v[dataIndex]); // to show updated freq
    Serial.print(v[dataIndex]);
    Serial.println(F(": value set"));
    // showFreq(v[dataIndex]);
    //setFreq(v[dataIndex], (int)dataIndex, true);
    //Serial.println(v[dataIndex]);
  }
}

//---------------- menu system -----------------------

int settingMax = 0;
int settingMin = -100;
int settingAttenuate = 0;
int settingGenerate = 0;
int settingBandwidth = 600;
int settingLevelCal = 0;
int settingPowerCal = 1;
 


#ifdef USE_ROTARY

const char T_off[]  PROGMEM = "Off";
const char T_on[]   PROGMEM = "On";

const char* const toggleText[] PROGMEM = {
  T_off,  //0
  T_on
};


#define MA_CC 1
#define MA_CZ 2
#define MA_IC 3
#define MA_IZ 4
#define MA_FR 5

#define menuItem   1
#define menuAction  2
#define menuValue   3
#define menuToggle  4

typedef struct  {
  const char kind;
  const char prev;
  const char next;
  const char up;
  const char menuMin;
  const char menuMax;
  const PROGMEM char itemText[10];
} menuType;


const menuType menu[] PROGMEM = {
#define MainMenu  1
  // 0
  {   menuItem,   0, 0, 0, MainMenu, 0, "Main"},
  // 1
  {  menuAction,MainMenu + 6, MainMenu + 1, 0, MA_FR, 0, "From",},
  {  menuAction,MainMenu + 0, MainMenu + 2, 0, MA_FR, 1, "To",},
  {  menuValue, MainMenu + 1, MainMenu + 3, 0, (char)-120, 20, "Max"},
  {  menuValue, MainMenu + 2, MainMenu + 4, 0, (char)-120, 20, "Min"},
  {  menuValue, MainMenu + 3, MainMenu + 5, 0, (char)-30, 0, "Attenuate"},
  {  menuToggle,MainMenu + 4, MainMenu + 6, 0, 0, 1, "Generate"},
  {  menuAction,MainMenu + 5, MainMenu + 7, 0, MA_FR, 2, "IF",},
  {  menuValue, MainMenu + 6, MainMenu + 8, 0, (char)-50, +50, "LevelCal",},
  {  menuValue, MainMenu + 7, MainMenu + 0, 0, (char)-1, 8, "PowerCal",},
#if 0
#endif
};


int currentMenu;
int menuActive;
//int dataIndex;
int dirty;
int stateChange;
int menuOpen;

void ProcessMenu(int *v = NULL);

void HandleMenu()
{
  switch (currentMenu) {
  case 3: ProcessMenu (&settingMax); break;
  case 4: ProcessMenu (&settingMin); break;
  case 5: ProcessMenu (&settingAttenuate);     
//    Serial.print(F("Attenuate = "));
//    Serial.println(settingAttenuate);
  break;
  case 6: ProcessMenu (&settingGenerate); break;
  case 8: ProcessMenu (&settingLevelCal); break;
  case 9: ProcessMenu (&settingPowerCal); break;
  default:if (menuActive && stateChange) showFreq(lFreq[dataIndex]); ProcessMenu ((int *)lFreq); break;
  }
  buttonEvent = 0;
}

void InitMenu()
{
  currentMenu = 1;
  menuActive = false;
  dataIndex = 0;
  dirty = false;
  stateChange = true;
  menuOpen = false;
}
char buffer[16];

void ProcessMenu(int *v)
{
  int kind; int n; int p; int u; int mi; 
  int ma; 

update:

  kind = pgm_read_byte(&(menu[currentMenu].kind));
  n = pgm_read_byte(&(menu[currentMenu].next));
  p = pgm_read_byte(&(menu[currentMenu].prev));
  u = pgm_read_byte(&(menu[currentMenu].up));
  mi = (char) pgm_read_byte(&(menu[currentMenu].menuMin)); if (mi > 127) mi  -= 256;
  ma = (char) pgm_read_byte(&(menu[currentMenu].menuMax)); if (ma > 127) ma  -= 256;

  if (stateChange || dirty) {
/*
    if (menuOpen)
      Serial.print(F("Open "));
    if (menuActive)
      Serial.print(F("Active "));
    if (dirty)
      Serial.print(F("Dirty "));
    if (stateChange)
      Serial.print(F("stateChange "));
    Serial.print(u);
    Serial.print(F(":["));
    Serial.print(p);
    Serial.print(F(":"));
    Serial.print(n);
    Serial.print(F("]{min="));
    Serial.print(mi);
    Serial.print(F(",max="));
    Serial.print(ma);
    Serial.print(F("}:"));
    Serial.print(currentMenu);
    Serial.println(F(": Menu entered"));
*/
  }

//#endif

#ifdef USE_DISPLAY
  __FlashStringHelper *ut;
  ut = ( __FlashStringHelper *) &(menu[u].itemText[0]);
  __FlashStringHelper *t;
  t = ( __FlashStringHelper *) &(menu[currentMenu].itemText[0]);

  if (stateChange || dirty) {
  tft.fillRect(0, 0, tft.width(), oY-2, DISPLAY_BLACK);
//    clearDisplay();
    textWhite();        // Draw white text
    tft.setCursor(0, 0);
    if (!menuActive) {
      tft.print(ut);
      tft.print(':');
      tft.print(t);   // Show menu text
    } else {
      tft.print(t);
      tft.print(':');
      //      tft.setCursor(8, 0);
      if (kind == menuToggle) {
        strcpy_P(buffer, (char*)pgm_read_word(&(toggleText[(v[dataIndex]) + mi]))); // Toggle text
        tft.print(buffer);
      } else if (kind == menuValue) {
        tft.print(v[dataIndex]);    // Value
      } else if (kind == menuAction) {
        //      tft.print((const __FlashStringHelper*)&(v[5])); // Nothing to show
      }
      // else do not show anything
    }
    sendDisplay();
    stateChange = false;
    dirty = false;
//    if (buttonEvent == 0)
//      return;
  }
#endif
  // buttonUp, buttonDown, buttonLongDown, buttonBack, buttonRotateUp, buttonRotateDown

//#ifdef USE_ROTARY
  
  if (!menuActive) {
    if (buttonEvent == buttonRotateUp) {
      currentMenu = n;
      stateChange = true;
    } else if (buttonEvent == buttonRotateDown) {
      currentMenu = p;
      stateChange = true;
    } else if (buttonEvent == shortBackClickRelease) {
      if (u != 0) {
        currentMenu = u + dataIndex;
        dataIndex = 0;
        stateChange = true;
      }
    } else if (buttonEvent == shortClickRelease) { // Single click
      if (kind == menuItem) {
        currentMenu = mi;
        dataIndex = ma;
      } else {
        if (kind == menuAction)
          dataIndex = ma;
        menuActive = true;
//        Serial.print(currentMenu);
//        Serial.println(F(": Menu activated"));
      }
      stateChange = true;
    }
    buttonEvent = 0;
  } else { // Menu is active
    if (buttonEvent == shortClickRelease) { // Single click
      menuActive = false;
      stateChange = true;
//      Serial.print(currentMenu);
//      Serial.println(F(": Menu deactivated"));
      buttonEvent = 0;
//      Serial.println(F("Menu Closed"));
      menuOpen = false; // Stop displaying menu
    } else 
    if (kind == menuToggle || kind == menuValue) {
      if (buttonEvent == buttonRotateDown && v[dataIndex] > mi) {
        v[dataIndex] -= (kind == menuToggle? 1 : 1);
        dirty = true;
      } else if (buttonEvent == buttonRotateUp && v[dataIndex] < ma) {
        v[dataIndex] += (kind == menuToggle? 1 : 1);
        dirty = true;
      }
      if (buttonEvent) {
//        Serial.print(v[dataIndex]);
//        Serial.println(F(": Value changed"));
      }
      buttonEvent = 0;
    } else if (kind == menuAction) {
      ChangeFrequency((long int *)v);
      buttonEvent = 0;
    }
  }
//#endif
}


#endif
//------------------------------------------


int debug = 0;


#define DebugLine(X) { if (debug) Serial.println(X); }
#define Debug(X) { if (debug) Serial.print(X); }


int inData = 0;
long steps = DISPLAY_POINTS;
unsigned long  startFreq = 250000000;
unsigned long  stopFreq = 300000000;
unsigned long  lastFreq[6] = { 300000000, 300000000,0,0,0,0};
int lastParameter[10];
int parameter;
int VFO = 0;
int RX = 2;
int extraVFO=-1;
int extraVFO2 = -1;
unsigned long reg = 0;
long offset=0;
long offset2=0;
static unsigned int spacing = 10000;
double delta=0.0;
int phase=0;
int deltaPhase;
int delaytime = 50;
int drive = 6;
unsigned int sensor;
int hardware = 0;


// A Arduino zero benefits from a large serial buffer, for a nano you can reduce the buffer size such as 64
#define BUFFERSIZE 256
uint8_t serialBuff[BUFFERSIZE];
volatile int     serialIndex=0;
//#define HIGHPOINT (BUFFERSIZE - 20)
#define HIGHPOINT (0)

void serialFlushIf(int amount)
{
  if (serialIndex > amount)
  {
    pinMode(tinySA_led, OUTPUT); // Flash led if serial data is being send
    digitalWrite(tinySA_led, HIGH);
    Serial.write(serialBuff, serialIndex);
    digitalWrite(tinySA_led, LOW);
    serialIndex = 0;
  }
}




void info()
{
  Serial.println("SI4432 Sweeper");
  Serial.print("A = Start frequency, currently ");
  Serial.println(startFreq);
  Serial.print("B = Stop frequency, currently ");
  Serial.println(stopFreq);
  Serial.print("S = Steps, currently ");
  Serial.println(steps);
  Serial.println("M = Single sweep");
  Serial.println("C = Continious sweep until Q");
  Serial.println("H = show menu");
  Serial.print("T = Timestep in ms, currently ");
  Serial.println(delaytime);
  Serial.print("O = Output frequency, currently ");
  Serial.println(lastFreq[VFO]);
//  Serial.print("D = Drive[2,4,6,8], currently ");
//  Serial.println(drive);
  Serial.print("V = VFO[0,1,2], currently ");
  Serial.println(VFO);
//  Serial.print("G = spacing, currently ");
//  Serial.println(spacing);
//  Serial.print("E = extra VFO and Offset, currently ");
//  Serial.print(extraVFO);
//  Serial.print("=");
//  Serial.print(offset);
//  Serial.print(", ");
//  Serial.print(extraVFO2);
//  Serial.print("=");
//  Serial.println(offset2);
//  Serial.print("F = Perform VNA scan: mode, startFreq, steps, freqStep, stepTime, IF, HW");
  Serial.print("X = read write hex register, last written 0x");
  Serial.println(reg, HEX);
//  Serial.print("Y = write stepper(+,-,/,* or delta), delta=");
//  Serial.println(delta);
  Serial.print("W = Set Bandwidth, currently ");
  Serial.println(bandwidth);
  Serial.print("? = Debug level ");
  Serial.println(debug);
//  Serial.println("R = Reset");
}


unsigned char myData[DISPLAY_POINTS+1]; 
int peakLevel;
double peakFreq;

#define BARSTART  24

#ifdef USE_DISPLAY

void displayHisto ()
{
//  clearDisplay();
  DrawCheckerBoard();
//int settingMax = 0;
//int settingMin = -120;
  int delta=settingMax - settingMin;

  // Display levels
  tft.fillRect(0, 0, oX-2, tft.height(), DISPLAY_BLACK);
  textWhite();
  tft.setCursor(0,oY);             // Start at top-left corner
  tft.println(settingMax);
  tft.setCursor(0,tft.height() - 8);
  tft.println(settingMin);
  tft.setCursor(0,tft.height()/2);
  tft.println("dB");

  // Dsiplay frequencies
  tft.fillRect(0, 0, tft.width(), oY-2, DISPLAY_BLACK);
  tft.setTextColor(DISPLAY_INVERSE);        // Draw white text
  tft.setCursor(oX+2,0);             // Start at top-left corner
  double f = (((double)(startFreq - lastFreq[0]))/ 1000000.0);
  tft.println(f);
  tft.setCursor(tft.width() - 36,0);
  f = (((double)(stopFreq - lastFreq[0]))/ 1000000.0);
  tft.println(f);
  tft.setCursor(tft.width()/2 - 18,0);
  tft.println("MHz");

  if (peakLevel > -150) {
    tft.setCursor(oX + 2,8);             // Start at top-left corner
    tft.print("Peak is = ");
    tft.print((int)((peakLevel/ 2.0  - settingAttenuate) - 120.0)+settingLevelCal);
    tft.print("dB, ");
    tft.print(peakFreq/ 1000000.0);
    tft.print("MHz");
  }

    
#define Y_GRIDS 9
  for (int i=0; i<DISPLAY_POINTS - 1; i++) {
    double f = ((myData[i] / 2.0  - settingAttenuate) - 120.0);
    f = (f - settingMin) * Y_GRIDS * dY / delta;
    if (f >= Y_GRIDS * dY) f = Y_GRIDS * dY-1;
    if (f < 0) f = 0;
    double f2 = ((myData[i+1] / 2.0  - settingAttenuate) - 120.0);
    f2 = (f2 - settingMin) * Y_GRIDS * dY / delta;
    if (f2 >= Y_GRIDS * dY) f2 = Y_GRIDS * dY-1;
    if (f2 < 0) f2 = 0;
  int x = i;
  int Y1 = Y_GRIDS * dY - 1 - (int)f;
  int Y2 = Y_GRIDS * dY - 1 - (int)f2;
  tft.drawLine(x+oX, oY+Y1, x+oX+1, oY+Y2, DISPLAY_YELLOW);
  tft.drawLine(x+oX, oY+Y1+1, x+oX+1, oY+Y2+1, DISPLAY_YELLOW);
  }
 


  sendDisplay();
}
#endif

void setup() 
{

  Serial.begin(115200); // 115200
#if defined(ARDUINO_ARCH_SAMD)
//  while(!SerialUSB); // Uncomment this line if you want the Arduino to wait with starting till the serial monitor is activated, usefull when debugging
#endif
//return;
#if defined(ARDUINO_ARCH_SAMD) 
  Wire.setClock(800000);
#endif

//SPISettings(12000000, MSBFIRST, SPI_MODE0)
//SPI.beginTransaction();
  
  Serial.println("Starting");
  SI4432_Init();
  Serial.println("Init done");

  info();

#ifdef USE_SSD1306
  if(!tft.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));

  }
#endif
#ifdef USE_ILI9341
  tft.begin();
  tft.setRotation(1);

#endif
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
//  sendDisplay();
//  displayHisto();
#ifdef USE_ROTARY
  R_setup();
#define ENABLE_PULLUPS 1
  pinMode(buttonPin, (ENABLE_PULLUPS ? INPUT_PULLUP : INPUT)); 
  pinMode(backButtonPin, (ENABLE_PULLUPS ? INPUT_PULLUP : INPUT)); 
  InitMenu();
#endif
  PE4302_init();
  PE4302_Write_Byte(0);
#ifdef USE_SI4463
  Si446x_init(); 
  Si446x_RX ((uint8_t)70);
#endif
}



void histo(int lev)
{
  Serial.print(lev);
//  Serial.print(": ");
 // while (lev > 80) lev = lev / 2;  
//  while (lev--)
//    Serial.print("*");
  Serial.println("");
}

long old_time = 0;

void setFreq(int V, unsigned long freq)
{
  if (V>=0) {
    SI4432_Sel = V;
#ifdef USE_SI4463
    if (SI4432_Sel == 2) {
      freq = freq - 433000000;
      freq = freq / 10000;  //convert to 10kHz channel starting with 433MHz
//      Serial.print("Set frequency Si4463 = ");
//      Serial.println(freq);
      Si446x_RX ((uint8_t)freq);
    }
    else
#endif
      SI4432_Set_Frequency(freq);
  }
}

int autoSweepStep = 0;
long autoSweepFreq = 0;
long autoSweepFreqStep = 0;
int standalone = true;

void loop()
{

#ifdef USE_ROTARY

  if( /*standalone && */ Serial.available() == 0) // Serial has priority
  {
//-------------------button handling --------------------
  int x = R_delta();
  int reading = digitalRead(buttonPin);
  int reading2 = digitalRead(backButtonPin);
  if (reading != lastButtonRead || reading2 != lastBackButtonRead ) {
    lastDebounceTime = millis();
  }
  lastButtonRead = reading;
  lastBackButtonRead = reading2;

  buttonEvent = 0;
  if ((millis() - lastDebounceTime) > debounceDelay) {
    switch(buttonState) {
    case buttonUp: 
      if (reading == LOW) 
      {
        buttonState = buttonDown;
      }
      break;
    case buttonDown: 
      if (reading == HIGH)
      {
        buttonState = buttonUp;
        buttonEvent = shortClickRelease;
      }
      if ((millis() - lastDebounceTime) > longPressDelay) 
      {
        buttonState = buttonLongDown;
        buttonEvent = longClick;
      }
      break;
    case buttonLongDown: 
      if (reading == HIGH)
      {
        buttonState = buttonUp;
        buttonEvent = longClickRelease;
      }
      break;

    }
    switch(backButtonState) {
    case buttonUp: 
      if (reading2 == LOW) 
      {
        backButtonState = buttonDown;
      }
      break;
    case buttonDown: 
      if (reading2 == HIGH)
      {
        backButtonState = buttonUp;
        buttonEvent = shortBackClickRelease;
      }
      if ((millis() - lastDebounceTime) > longPressDelay) 
      {
        backButtonState = buttonLongDown;
        buttonEvent = longBackClick;
      }
      break;
    case buttonLongDown: 
      if (reading2 == HIGH)
      {
        backButtonState = buttonUp;
        buttonEvent = longBackClickRelease;
      }
      break;

    }
  }
  if (x) 
  {
    Serial.print("Rotaryevent: ");
    Serial.println(x);
    long new_time = millis();
    if (new_time - old_time > 50)
      incr = 1;
    else if (new_time - old_time > 20)
      incr = 5;
    else
      incr = 20;
    old_time = new_time;
    if (x > 0)
      buttonEvent = buttonRotateUp;
    else
      buttonEvent = buttonRotateDown;
  }  
  if (buttonEvent != 0) {
 //   Serial.print("Buttonevent: ");
 //   Serial.println(buttonEvent);
  }
  if (buttonEvent || stateChange|| dirty) {
    if (buttonEvent && !menuOpen) {
      menuOpen = true;
      buttonEvent = 0;
      menuActive = false;
      dataIndex = 0;
      dirty = true;
      stateChange = true;
//      Serial.println(F("Menu open"));
    } 
    HandleMenu();
  }

  if (!menuOpen && standalone) {
//    Serial.print("AutoSweepStep: ");
//    Serial.println(autoSweepStep);
    if (autoSweepStep == 0) {
      autoSweepFreq = lFreq[0];
      autoSweepFreqStep = (lFreq[1] - lFreq[0])/DISPLAY_POINTS;
      setFreq (0, lFreq[2]);
      lastFreq[0] = lFreq[2];
      startFreq = lFreq[0] + lFreq[2];
      stopFreq = lFreq[1] + lFreq[2];
      int p = - settingAttenuate * 2;
      PE4302_Write_Byte(p);
      SetPowerReference(settingPowerCal);
      settingBandwidth = ((float)(lFreq[1] - lFreq[0]))/DISPLAY_POINTS/1000.0;
      SI4432_Sel = 0;
      SI4432_SET_RBW(settingBandwidth);
      SI4432_Sel = 1;
      SI4432_Write_Byte(0x6D, 0x1C + (drive - 2 )/2);//Set full power

      peakLevel = -150;
      peakFreq = -1.0;
    }
    SI4432_Sel=1;
    setFreq (1, lFreq[2] + autoSweepFreq);
    SI4432_Sel=0;
    myData[autoSweepStep] = SI4432_RSSI();
    if (settingBandwidth > 500) {
      int subSteps = (settingBandwidth / 500) - 1;
      while (subSteps > 0) {
//Serial.print("substeps = ");
//Serial.println(subSteps);
       SI4432_Sel=1;
       setFreq (1, lFreq[2] + autoSweepFreq + subSteps * 500000);
//Serial.print("Freq = ");
//Serial.println(lFreq[2] + autoSweepFreq + subSteps * 500000);
       SI4432_Sel=0;
       int subRSSI = SI4432_RSSI();
       if (myData[autoSweepStep] < subRSSI)
         myData[autoSweepStep] = subRSSI;
        subSteps--;
      }
    }

    if (autoSweepFreq > 1000000) {
      if (peakLevel < myData[autoSweepStep]) {
        peakLevel = myData[autoSweepStep];
        peakFreq = autoSweepFreq;
      }
    }
    
    if (myData[autoSweepStep] == 0) {
        SI4432_Init();
    }
    autoSweepStep++;
    autoSweepFreq += (lFreq[1] - lFreq[0])/DISPLAY_POINTS;
    if (autoSweepStep >= DISPLAY_POINTS) {
      autoSweepStep = 0;
#if USE_DISPLAY
      displayHisto();
#endif
    }  
   }
  }
#endif
//--------------------------------  
  inData = 0;

//  if (!Serial) NVIC_SystemReset(); 
  if(Serial.available() > 0)   // see if incoming serial data:
  {
    inData = Serial.read();  // read oldest byte in serial buffer:
    // Serial.println(inData);

  if(inData == 'M' || inData == 'm' || inData == 'L' || inData == 'l')
  {
    standalone = false;
    double oldfreq, freq;
    int isLogSweep = false;
    double freqstep = (stopFreq - startFreq) / steps;
    double freqmult = pow(10.0, log10(stopFreq/startFreq)/steps);
    unsigned long old_micros, start_micros;
    unsigned long old_millis;

    if(inData == 'L' || inData == 'l') 
      isLogSweep = true;
    inData = 0;
    SI4432_Sel=RX;
    if (SI4432_RSSI() == 0) {
//        Serial.println("Init done");
        SI4432_Init();
    }

    delay(10);
    freq = startFreq;
    Serial.println("{");
    old_millis = millis();
    for(int i = 0; i < steps; i++ )
    {
      unsigned long modfreq = freq;
      serialFlushIf(HIGHPOINT);
      old_micros = micros();
      if (extraVFO>=0) {
        setFreq(extraVFO,modfreq-offset);
        lastFreq[extraVFO] = modfreq-offset;
      } 
      if (extraVFO2>=0) {
        setFreq(extraVFO2,modfreq-offset2);
        lastFreq[extraVFO2] = modfreq-offset2;
      } 
      setFreq (VFO, modfreq);
      lastFreq[VFO] = freq;
      if (i>0) {
        serialBuff[serialIndex++] = 'x'; 
        serialBuff[serialIndex++] = ((byte) (sensor));
        serialBuff[serialIndex++] = ((byte) (sensor>>8));
    serialFlushIf(0); ///TEMP -----------------------------------------------

        if (i < 128){
          myData[i] = sensor;
        }
      
      }
#if 1
      while (micros() - old_micros < (delaytime * 100L)*2/3 ) {
        delayMicroseconds(100);
      }

#endif
      oldfreq = freq;
      if (isLogSweep)
        freq = freq * freqmult;
      else
        freq = freq + freqstep;
//      if (bandwidth>0) {
//        sensor = (int)(millis() - old_millis);
//      } else { 
        SI4432_Sel=RX;
        sensor = SI4432_RSSI();
//      }
    }
    serialBuff[serialIndex++] = 'x'; 
    serialBuff[serialIndex++] = ((byte) (sensor));
    serialBuff[serialIndex++] = ((byte) (sensor>>8));
//    displayHisto();
    serialFlushIf(0);
    Serial.println("}");
    //standalone = true;
  }

  if(inData == 'S' || inData == 's')
  {
    steps = Serial.parseInt();
    Serial.print("Steps: ");
    Serial.println(steps);
  }

  if(inData == 'H' || inData == 'h')
  {
    info();
  }

  if(inData == 'X' || inData == 'x')
  {
      char t[40];
      int i = 0;
      int reg;
      int addr;
      char c = 0;
      delay(1);
      while (Serial.available() > 0 && c != ' ') {
        delay(1);
        c = Serial.read();  //gets one byte from serial buffer
        t[i++] = c; //makes the string readString
      }
      t[i++] = 0;
      addr = strtoul(t, NULL, 16);
      i = 0;
      while (Serial.available() > 0) {
        delay(1);
        c = Serial.read();  //gets one byte from serial buffer
        t[i++] = c; //makes the string readString
      }
      t[i++] = 0;
      SI4432_Sel = VFO;
      if (i == 1) {
        Serial.print("Reg[");
        Serial.print(addr, HEX);
        Serial.print("] : ");
        Serial.println(SI4432_Read_Byte(addr), HEX);
      } else {
        reg = strtoul(t, NULL, 16);
        Serial.print("Reg[");
        Serial.print(addr, HEX);
        Serial.print("] = ");
        Serial.println(reg, HEX);
        SI4432_Write_Byte(addr, reg);
      }
    inData = 0;
  }

  
  if(inData == '?')
  {
    debug = !debug;
    Serial.print("Debug level ");
    Serial.println(debug);
  }

  if(inData == 'T' || inData == 't')
  {
    delaytime = Serial.parseInt();
    Serial.print("time pr step: ");
    Serial.println(delaytime);
    inData = 0;
  }

  if(inData == 'A' || inData == 'a')
  {
    startFreq = Serial.parseInt();
    Serial.print("Start: ");
    Serial.println(startFreq);
    inData = 0;
  }

  if(inData == 'B' || inData == 'b')
  {
    stopFreq = Serial.parseInt();
    Serial.print("Stop: ");
    Serial.println(stopFreq);
    inData = 0;
  }

  if(inData == 'O' || inData == 'o')
  {
    lastFreq[VFO] = Serial.parseInt();
    unsigned long modfreq = lastFreq[VFO];
    setFreq(VFO,modfreq);
    if (extraVFO>=0) {
        setFreq(extraVFO,modfreq-offset);
    }
    if (extraVFO2>=0) {
        setFreq(extraVFO2,modfreq-offset2);
    }
    Serial.print("Output frequency: ");
    Serial.println(modfreq);
    inData = 0;
  }

  if(inData == 'Y' || inData == 'y')
  {
    if(Serial.available() > 0)   // see if incoming serial data:
    {
      inData = Serial.read();  // read oldest byte in serial buffer:

      if (inData == '+') {
        lastFreq[VFO] += delta;
      } else
      if (inData == '-') {
        lastFreq[VFO] -= delta;
      } else
      if (inData == '*') {
        lastFreq[VFO] *= delta;
      } else
      if (inData == '/') {
        lastFreq[VFO] /= delta;
      } else {
        delta = Serial.parseFloat();
      }
      unsigned long modfreq = lastFreq[VFO];
      setFreq(VFO,modfreq);
      if (extraVFO>=0) {
         setFreq(extraVFO,modfreq-offset);
      }
      if (extraVFO2>=0) {
        setFreq(extraVFO2,modfreq-offset2);
      }
//      Serial.print("Output frequency: ");
//      Serial.println(modfreq);
      inData = 0;
    }

  }
  if(inData == 'E' || inData == 'e')
  {
    if(Serial.available()) {
     extraVFO = Serial.parseInt();
     Serial.print("Extra VFO: ");
     Serial.println(extraVFO);
     if (extraVFO>=0) {
        offset = Serial.parseInt();
        Serial.print("Offset: ");
        Serial.println(offset);
        if(Serial.available()) {
          extraVFO2 = Serial.parseInt();
          Serial.print("Extra VFO2: ");
          Serial.println(extraVFO2);
          if (extraVFO2>=0) {
              offset2 = Serial.parseInt();
              Serial.print("Offset2: ");
              Serial.println(offset2);
          }
        }
        else
        {
          extraVFO2 = -1;
        }
     }
    } 
    else{ 
      extraVFO=-1;
      extraVFO2=-1;
    }
   inData = 0;
  }

  if(inData == 'D' || inData == 'd')
  {
    drive = Serial.parseInt();
    Serial.print("Drive: ");
    Serial.println(drive);
    inData = 0;
  }
  if(inData == 'V' || inData == 'v')
  {
    VFO = Serial.parseInt();
    Serial.print("VFO: ");
    Serial.println(VFO);
    inData = 0;
  }
  if(inData == 'G' || inData == 'g')
  {
    spacing = Serial.parseInt();
    Serial.print("Spacing: ");
    Serial.println(spacing);
    inData = 0;
  }
  if(inData == 'W' || inData == 'w')
  {
    if(Serial.available()) {
      bandwidth = Serial.parseFloat();
      settingBandwidth = (int)bandwidth;
      Serial.print("Width: ");
      Serial.println(spacing);
    } else 
      bandwidth = 300.0;
    SI4432_Sel = RX;
    SI4432_SET_RBW(bandwidth);
    inData = 0;
  }
  if(inData == 'N' || inData == 'n')
  {
    bandwidth = 30.0;
    SI4432_Sel = RX;
    SI4432_SET_RBW(bandwidth);
    inData = 0;
  }
  if(inData == 'P' || inData == 'p')
  {
    if(Serial.available()) {
     parameter = Serial.parseInt();
     if(Serial.available()) {
          lastParameter[parameter] = Serial.parseInt();

      if (parameter == 0) {
//        ADF4351_spur_mode(lastParameter[0]);
//        ADF4351_Set(VFO);
      } else if (parameter == 1) {
        SetPowerReference(lastParameter[1]);
#if 0
          SI4432_Sel = 1;         //Select Lo module
          if (lastParameter[1] < 0 || lastParameter[1] > 7 ) {
            SI4432_Write_Byte(0x0D, 0x1F) ; // Set GPIO2 to GND
          } else {
            SI4432_Write_Byte(0x0D, 0xC0) ; // Set GPIO2 maximumdrive and clock output
            SI4432_Write_Byte(0x0A, lastParameter[1] & 0x07) ; // Set GPIO2 frequency
          }
#endif
//        ADF4351_R_counter(lastParameter[1]);
//        ADF4351_Set(VFO);
      } else if (parameter == 2) {
//        ADF4351_channel_spacing(lastParameter[2]);
//        ADF4351_Set(VFO);
      } else if (parameter == 3) {
//        ADF4351_CP(lastParameter[3]);
//        ADF4351_Set(VFO);
      } else if (parameter == 4) {
//        ADF4351_level(lastParameter[4]);
//        ADF4351_Set(VFO);
      } else if (parameter == 5) {
        settingAttenuate = lastParameter[5];
        int p = - settingAttenuate * 2;
        PE4302_Write_Byte(p);
      } else if (parameter == 6) {
        RX = lastParameter[6];

        if (RX == 3) {  //Both on RX
          SI4432_Sel = 0;
          SI4432_Write_Byte(0x7, 0x0B); // start TX
          SI4432_Write_Byte(0x6D, 0x1F);//Set low power
          SI4432_Sel = 1;
          SI4432_Write_Byte(0x7, 0x0B); // start TX
          SI4432_Write_Byte(0x6D, 0x1F);//Set full power
        } else {
          if (RX == 0) {
            SI4432_Sel = 0;
            SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain

            SI4432_Sel = 1;
            SI4432_Write_Byte(0x7, 0x0B); // start TX
            SI4432_Write_Byte(0x6D, 0x1C + (drive - 2 )/2);//Set full power
            
          } else if (RX == 1) {
            SI4432_Sel = 0; // both as receiver to avoid spurs
            SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain

            SI4432_Sel = 1;
            SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain
            
          } else if (RX == 2) { // SI4463 as receiver
            SI4432_Sel = 0;
            SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain

            SI4432_Sel = 1;
            SI4432_Write_Byte(0x7, 0x0B); // start TX
            SI4432_Write_Byte(0x6D, 0x1C + (drive - 2 )/2);//Set full power
          }
#if 0 // compact
          SI4432_Sel = (RX ? 1 : 0);
          SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain

          SI4432_Sel = (RX ? 0 : 1);
          SI4432_Write_Byte(0x7, 0x0B); // start TX
          SI4432_Write_Byte(0x6D, 0x1C + (drive - 2 )/2);//Set full power
#endif
        }
      }
     }
     Serial.print("Parameter  ");
     Serial.print(parameter);
     Serial.print(" = ");
     Serial.println(lastParameter[parameter]);

    } 
    else{ 
      parameter=-1;
    }
   inData = 0;
  }

  if(inData == 'R' || inData == 'r')
  {
      int cont = 1;
      while (cont) {
        float rssi = SI4432_RSSI();
        Serial.println(rssi, DEC);
        if (Serial.available() > 0) {
          inData = Serial.read();  // read oldest byte in serial buffer:
          if (inData == 'q') 
            cont = 0;
        }
      }
      Serial.println("Stopped"); // Home
  }
  }
}
