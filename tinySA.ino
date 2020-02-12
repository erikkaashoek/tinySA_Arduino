/* Copyright (c) 2017 Erik Kaashoek <erik@kaashoek.com>
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



//#if defined(ARDUINO_ARCH_SAMD) 
#define  Serial SerialUSB
//#endif

// Comment out below line if you do not want a local user interface
#define USE_DISPLAY 1
// #define USE_SSD1306 1
#define USE_ILI9341 1
// #define USE_SI4463  1
//#define USE_ILI9488 1

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
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_DC A5
#define TFT_CS A4

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define DISPLAY_POINTS 290

#endif

//#define SCREEN_WIDTH 320 // OLED display width, in pixels
//#define SCREEN_HEIGHT 240 // OLED display height, in pixels



#ifdef USE_ILI9488
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "ILI9488.h"

// For the Adafruit shield, these are the default.
#define TFT_DC A5
#define TFT_CS A1
#define TFT_RST A0

ILI9488 tft = ILI9488(TFT_CS, TFT_DC, TFT_RST);

#define DISPLAY_POINTS 440

#endif

// The onboard led is blinked during serial transfer
#define tinySA_led 13

#ifndef DISPLAY_POINTS
#define DISPLAY_POINTS 100
#endif

//--------------------- Generic display ---------------


#ifdef USE_SSD1306  
#define DISPLAY_WHITE SSD1306_WHITE
#define DISPLAY_BLACK SSD1306_BLACK
#define DISPLAY_GRAY SSD1306_WHITE
#define DISPLAY_YELLOW SSD1306_WHITE
#define DISPLAY_INVERSE SSD1306_INVERSE
#endif
#ifdef USE_ILI9341
#define DISPLAY_WHITE ILI9341_WHITE
#define DISPLAY_BLACK ILI9341_BLACK
#define DISPLAY_DARKGREY ILI9341_DARKGREY
#define DISPLAY_YELLOW ILI9341_YELLOW
#define DISPLAY_ORANGE ILI9341_ORANGE
#define DISPLAY_RED ILI9341_RED
#define DISPLAY_GREEN ILI9341_GREEN
#define DISPLAY_BLUE ILI9341_BLUE
#define DISPLAY_INVERSE  ILI9341_WHITE
#define SCREEN_WIDTH 320 // OLED display width, in pixels
#define SCREEN_HEIGHT 240 // OLED display height, in pixels
#endif
#ifdef USE_ILI9488
#define DISPLAY_WHITE ILI9488_WHITE
#define DISPLAY_BLACK ILI9488_BLACK
#define DISPLAY_DARKGREY ILI9488_DARKGREY
#define DISPLAY_YELLOW ILI9488_YELLOW
#define DISPLAY_INVERSE  ILI9488_WHITE
#define SCREEN_WIDTH 320 // OLED display width, in pixels
#define SCREEN_HEIGHT 240 // OLED display height, in pixels
#endif

void clearDisplay()
{
#ifdef USE_SSD1306  
  tft.clearDisplay();
#endif
#ifdef USE_ILI9341
  tft.fillScreen(DISPLAY_BLACK);
#endif
#ifdef USE_ILI9488
  tft.fillScreen(DISPLAY_BLACK);
#endif
}

void textWhite()
{
#ifdef USE_SSD1306  
   tft.setTextSize(1);
   tft.setTextColor(SSD1306_WHITE);        // Draw white text
#endif
#ifdef USE_ILI9341
  tft.setTextColor(DISPLAY_WHITE); 
#endif
#ifdef USE_ILI9488
  tft.setTextColor(DISPLAY_WHITE); 
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
#ifdef USE_ILI9488
 //   yield();
#endif
}

//---------------- Drawing ------------------

#if USE_ILI9341 || USE_ILI9488

const int char_height = 8;

#define X_GRID  10
#define Y_GRID  9

#if USE_ILI9341
const int dX = 29 ;
const int dY = char_height * 3 ;
const int oX = 27 ;
const int oY = char_height * 2 -1 ;
#endif

#if USE_ILI9488
const int dX = 44 ;
const int dY = 32;
const int oX = 20 ;
const int oY = char_height * 2 ;
#endif

/*
void DrawCheckerBoard() 
{

  // VERTICAL
  for (int y=0; y<Y_GRID; y++)
  {
    // HORIZONTAL
    for (int x=0; x<X_GRID; x++)
    {
      tft.drawRect(oX + x*dX, y*dY+oY, dX, dY, DISPLAY_DARKGREY);
      tft.fillRect(oX + x*dX+1, y*dY+1+oY, dX-2, dY-2, DISPLAY_BLACK);
    }
    tft.drawRect(oX, oY, X_GRID * dX, Y_GRID * dY, DISPLAY_DARKGREY);  
  }
  tft.drawRect(oX, oY, X_GRID * dX, Y_GRID * dY, DISPLAY_DARKGREY);  
}
*/

int oldY[DISPLAY_POINTS+1]; 

void AllDirty()
{
  for (int i=0; i<DISPLAY_POINTS;i++) oldY[i] = 0;
//  Serial.println("Grids reset");
}

void MenuDirty()
{
  for (int i=200; i<DISPLAY_POINTS;i++) oldY[i] = 0;
//  Serial.println("Grids reset");
}


void DrawCheckerBoard(int x) 
{
  int y=oldY[x];
  if ((x % dX) != 0) {
    tft.drawLine(oX+x, oY+y, oX+x, oY+Y_GRID * dY, DISPLAY_BLACK);  
    for (y=0; y<=Y_GRID; y++)
    {
      tft.drawPixel(oX+x, y*dY+oY, DISPLAY_DARKGREY);
    }
  }
  else {
    tft.drawLine(oX+x, oY+y, oX+x, oY+Y_GRID * dY, DISPLAY_DARKGREY);  
  }  
  if (x == X_GRID*dX-1)
    tft.drawLine(oX+X_GRID*dX, oY+y, oX+X_GRID*dX, oY+Y_GRID * dY, DISPLAY_DARKGREY);  
  oldY[x] = Y_GRID*dY; // Clean
  return;
}

void DrawDirty(int x,int y)
{
  if (x>=DISPLAY_POINTS)
    return;
  if (oldY[x] > y) {
    oldY[x] = y;
//    Serial.print("oldY[");
//    Serial.print(x);
//    Serial.print("] changed to ");
//    Serial.println(oldY[x]);
  }
}
#endif

//------------------ On screen menu system ---------------------

#include "ui.h"

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


//  SI4432_Write_Byte(0x69, 0x00); // No AGC, min LNA
//  SI4432_Write_Byte(0x69, 0x10); // No AGC, max LNA of 20dB
//  SI4432_Write_Byte(0x69, 0x20); // AGC, min LNA
  SI4432_Write_Byte(0x69, 0x60); // AGC, min LNA, Gain increase during signal reductions
//  SI4432_Write_Byte(0x69, 0x30); // AGC, max LNA
//  SI4432_Write_Byte(0x69, 0x70); // AGC, max LNA, Gain increase during signal reductions

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
#ifdef USE_ROTARY
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
#endif
// ---------------------------------------------------

#define MAX_VFO 3
long lFreq[MAX_VFO] = { 0,100000000,433700000};
int dataIndex = 0;


//--------------------- Frequency control -----------------------

int dirty = true;

#define STOP_MAX 430000000
#define START_MIN 0

int32_t frequency0 = 0;
int32_t frequency1 = 100000000;

static void update_frequencies(void)
{
//  chMtxLock(&mutex_sweep);
  uint32_t start, stop;
  if (frequency1 > 0) {
    start = frequency0;
    stop = frequency1;
  } else {
    int32_t center = frequency0;
    int32_t span = -frequency1;
    start = center - span/2;
    stop = center + span/2;
  }
  lFreq[0] = start;
  lFreq[1] = stop;
//  set_frequencies(start, stop, sweep_points);
//  operation_requested = OP_FREQCHANGE;
  
//  update_marker_index();
  
  // set grid layout
//  update_grid();
//  chMtxUnlock(&mutex_sweep);
}

static void freq_mode_startstop(void)
{
  if (frequency1 <= 0) {
    int center = frequency0;
    int span = -frequency1;
 //   ensure_edit_config();
    frequency0 = center - span/2;
    frequency1 = center + span/2;
  }
}

static void freq_mode_centerspan(void)
{
  if (frequency1 > 0) {
    int start = frequency0;
    int stop = frequency1;
//    ensure_edit_config();
    frequency0 = (start + stop)/2; // center
    frequency1 = -(stop - start); // span
  }
}


void set_sweep_frequency(int type, int32_t freq)
{
//  chMtxLock(&mutex_sweep);
  int32_t center;
  int32_t span;
//  int cal_applied = cal_status & CALSTAT_APPLY;
  dirty = true;
  switch (type) {
  case ST_START:
//    ensure_edit_config();
    freq_mode_startstop();
    if (freq < START_MIN)
      freq = START_MIN;
    if (freq > STOP_MAX)
      freq = STOP_MAX;
    frequency0 = freq;
    // if start > stop then make start = stop
    if (frequency1 < freq)
      frequency1 = freq;
    update_frequencies();
    break;
  case ST_STOP:
//    ensure_edit_config();
    freq_mode_startstop();
    if (freq > STOP_MAX)
      freq = STOP_MAX;
    if (freq < START_MIN)
      freq = START_MIN;
    frequency1 = freq;
    // if start > stop then make start = stop
    if (frequency0 > freq)
      frequency0 = freq;
    update_frequencies();
    break;
  case ST_CENTER:
//    ensure_edit_config();
    freq_mode_centerspan();
    if (freq > STOP_MAX)
      freq = STOP_MAX;
    if (freq < START_MIN)
      freq = START_MIN;
    frequency0 = freq;
    center = frequency0;
    span = -frequency1;
    if (center-span/2 < START_MIN) {
      span = (center - START_MIN) * 2;
      frequency1 = -span;
    }
    if (center+span/2 > STOP_MAX) {
      span = (STOP_MAX - center) * 2;
      frequency1 = -span;
    }
    update_frequencies();
    break;
  case ST_SPAN:
 //   ensure_edit_config();
    freq_mode_centerspan();
    if (freq > STOP_MAX-START_MIN)
        freq = STOP_MAX-START_MIN;
    if (freq < 0)
      freq = 0;
    frequency1 = -freq;
    center = frequency0;
    span = -frequency1;
    if (center-span/2 < START_MIN) {
      center = START_MIN + span/2;
      frequency0 = center;
    }
    if (center+span/2 > STOP_MAX) {
      center = STOP_MAX - span/2;
      frequency0 = center;
    }
    update_frequencies();
    break;
  case ST_CW:
//    ensure_edit_config();
    freq_mode_centerspan();
    if (freq > STOP_MAX)
      freq = STOP_MAX;
    if (freq < START_MIN)
      freq = START_MIN;
    frequency0 = freq;
    frequency1 = 0;
    update_frequencies();
    break;
  }

//  if (cal_auto_interpolate && cal_applied)
//    cal_interpolate(lastsaveid);
//  chMtxUnlock(&mutex_sweep);
}

uint32_t get_sweep_frequency(int type)
{
  if (frequency1 >= 0) {
    switch (type) {
    case ST_START: return frequency0;
    case ST_STOP: return frequency1;
    case ST_CENTER: return (frequency0 + frequency1)/2;
    case ST_SPAN: return frequency1 - frequency0;
    case ST_CW: return (frequency0 + frequency1)/2;
    }
  } else {
    switch (type) {
    case ST_START: return frequency0 + frequency1/2;
    case ST_STOP: return frequency0 - frequency1/2;
    case ST_CENTER: return frequency0;
    case ST_SPAN: return -frequency1;
    case ST_CW: return frequency0;
    }
  }
  return 0;
}



#ifdef USE_ROTARY

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
#endif
//---------------- data -----------------------


unsigned char myData[DISPLAY_POINTS+1]; 
unsigned char myStorage[DISPLAY_POINTS+1]; 
unsigned char myActual[DISPLAY_POINTS+1]; 

//---------------- menu system -----------------------

int settingMax = -10; //9 drids vertical
int settingMin = -100;
int settingAttenuate = 0;
int settingGenerate = 0;
int settingBandwidth = 0;
int settingLevelOffset = 0;
int settingPowerCal = 1;
int settingPowerGrid = 10;
int settingSpur = 0;
int settingAverage = 0;
int settingShowStorage = 0;
int settingSubtractStorage = 0;

void set_refer_output(int v)
{
  settingPowerCal = v;
  dirty = true;
}

void SetRefLevel(int ref)
{
  settingMin = ref - (settingMax - settingMin);
  settingMax =ref;
  dirty = true;
}

void SetGenerate(int g)
{
  settingGenerate = g;
  dirty = true;
}

void SetPowerGrid(int g)
{
  settingPowerGrid = g;
  settingMin = settingMax - 9*g;
  dirty = true;
}

void SetAttenuation(int a)
{
  settingAttenuate = -a;
  dirty = true;
}

void SetStorage(void)
{
  for (int i=0; i<DISPLAY_POINTS;i++)
    myStorage[i] = myData[i];
  settingShowStorage = true;
}

void SetClearStorage(void)
{
  settingShowStorage = false;
  settingSubtractStorage = false;
}

void SetSubtractStorage(void)
{
  if (!settingShowStorage)
    SetStorage();
  settingSubtractStorage = true;
}

extern int peakLevel;
void SetPowerLevel(int o)
{
  if (o != 100)
    settingLevelOffset = o - (int)((peakLevel/ 2.0  - settingAttenuate) - 120.0);
  else
    settingLevelOffset = 0;
  dirty = true;
}

void SetRBW(int v)
{
  settingBandwidth = v;
  dirty = true;
}

void SetSpur(int v)
{
  settingSpur = v;
  dirty = true;
}

void SetAverage(int v)
{
  settingAverage = v;
  dirty = true;
}


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


int peakLevel;
double peakFreq;
int peakIndex;

#define BARSTART  24

#ifdef USE_DISPLAY

static int old_settingAttenuate = -1000;
static int   old_settingPowerGrid = -1000;
static int   old_settingMax = -1;
static int   old_settingMin = -1;
static long    old_startFreq = -1;
static long    old_stopFreq = -1;
static int ownrbw = 0;
static int old_ownrbw = -1;
static int vbw = 0;
static int old_vbw = -1;
static int old_settingAverage = -1;
static int old_settingSpur = -100;

void redrawHisto() {
  old_settingAttenuate = -1000;
  old_settingPowerGrid = -1000;
  old_settingMax = -1;
  old_settingMin = -1;
   old_startFreq = -1;
   old_stopFreq = -1;
  old_settingAverage = -1;
  old_settingSpur = -100;
  AllDirty();
}

char *averageText[] = { "OFF", "MIN", "MAX", "2", "4", "8"};

void displayHisto ()
{
//  clearDisplay();
//int settingMax = 0;
//int settingMin = -120;

  if (old_settingMax != settingMax || old_settingMin != settingMin) {
  // Display levels at left of screen
  tft.fillRect(0, 0, oX-2, tft.height(), DISPLAY_BLACK);
  textWhite();
  tft.setCursor(0,oY);             // Start at top-left corner
  tft.println(settingMax);
  tft.setCursor(0,tft.height() - 16);
  tft.println(settingMin);
//  tft.setCursor(0,tft.height()/2);
//  tft.println("dB");
    old_settingMax = settingMax;
    old_settingMin = settingMin;
  }

  if (old_startFreq != startFreq || old_stopFreq != stopFreq) {
  // Dsiplay frequencies
// Bottom of screen
  tft.fillRect(0, tft.height()-8, tft.width(), tft.height()-1, DISPLAY_BLACK);
    tft.setTextColor(DISPLAY_WHITE);        // Draw white text
  tft.setCursor(oX+2,tft.height()-8);             // Start at top-left corner
  double f = (((double)(startFreq - lastFreq[0]))/ 1000000.0);
  tft.print(f);
  tft.print("MHz");
  tft.setCursor(tft.width() - 58,tft.height()-8);
  f = (((double)(stopFreq - lastFreq[0]))/ 1000000.0);
  tft.print(f);
  tft.print("MHz");

  tft.setCursor(tft.width()/2 - 80 + oX,tft.height()-8);
  tft.print("center:");
  f = (double)((stopFreq/2 + startFreq/2 - lastFreq[0]) / 1000000.0);
  tft.print(f);
  tft.print("MHz");
  old_startFreq = startFreq;
  old_stopFreq = stopFreq;
  }

// Top of screen

  if (old_settingAttenuate != settingAttenuate || old_settingPowerGrid != settingPowerGrid) {
  tft.fillRect(0, 0, 8*6, oY-2, DISPLAY_BLACK);
  tft.setCursor(0,0);             // Start at top-left corner
  tft.setTextColor(DISPLAY_WHITE);        // Draw white text
  tft.print("Atten:");
  tft.print(settingAttenuate);
  tft.setCursor(0,8);             // Start at top-left corner
  tft.print(settingPowerGrid);
  tft.print("dB/");
  old_settingAttenuate = settingAttenuate;
  old_settingPowerGrid = settingPowerGrid;
  old_ownrbw = -1;
  }  

  if (old_ownrbw != ownrbw || old_vbw != vbw) {
  tft.fillRect(56, 0, 99, oY-2, DISPLAY_BLACK);
  tft.setCursor(56,0);             // Start at top-left corner
  tft.setTextColor(DISPLAY_WHITE);        // Draw white text
  tft.print("RBW:");
  tft.print(ownrbw);
  tft.print("kHz");
  tft.setCursor(56,8);             // Start at top-left corner
  tft.print("VBW:");
  tft.print(vbw);
  tft.print("kHz");
  old_ownrbw = ownrbw;
  old_vbw = vbw;
  }  
  
  if (peakLevel > -150) {
    tft.fillRect(oX+100, 0, 100, 8-1, DISPLAY_BLACK);
    tft.setCursor(oX + 100,0);             // Start at top-left corner
    tft.setTextColor(DISPLAY_WHITE);        // Draw white text
    tft.print("Max=");
    tft.print((int)((peakLevel/ 2.0  - settingAttenuate) - 120.0)+settingLevelOffset);
    tft.print("dB, ");
    tft.print(peakFreq/ 1000000.0);
    tft.print("MHz");
  }

  if (old_settingAverage != settingAverage || abs(old_settingSpur) != abs(settingSpur)) {
    int x =  tft.width() - 60;
  tft.fillRect( x, 0, 60, oY-2, DISPLAY_BLACK);
  tft.setTextColor(DISPLAY_WHITE);        // Draw white text
  if (settingAverage) {
    tft.setCursor( x,0);             // Start at top-left corner
    tft.print("AVR:");
    tft.print(averageText[settingAverage]);
  }
  if (settingSpur) {
    tft.setCursor(x,8);             // Start at top-left corner
    tft.print("SPUR:");
    tft.print("ON");
  }
  old_settingAverage = settingAverage;
  old_settingSpur = settingSpur;
  }  


    
/*
  for (int i=0; i<DISPLAY_POINTS - 1; i++) {
    int delta=settingMax - settingMin;
    DrawCheckerBoard(i);
    double f = ((myData[i] / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
    f = (f - settingMin) * Y_GRID * dY / delta;
    if (f >= Y_GRID * dY) f = Y_GRID * dY-1;
    if (f < 0) f = 0;
    double f2 = ((myData[i+1] / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
    f2 = (f2 - settingMin) * Y_GRID * dY / delta;
    if (f2 >= Y_GRID * dY) f2 = Y_GRID * dY-1;
    if (f2 < 0) f2 = 0;
  int x = i;
  int Y1 = Y_GRID * dY - 1 - (int)f;
  int Y2 = Y_GRID * dY - 1 - (int)f2;
  tft.drawLine(x+oX, oY+Y1, x+oX+1, oY+Y2, DISPLAY_YELLOW);
//  tft.drawLine(x+oX, oY+Y1+1, x+oX+1, oY+Y2, DISPLAY_YELLOW);
  }
 

*/
  sendDisplay();
}

void DisplayPoint(unsigned char *data, int i, int color)
{
    if (i == 0)
      return;
    int x = i-1;
    int delta=settingMax - settingMin;
    double f = ((data[x] / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
    f = (f - settingMin) * Y_GRID * dY / delta;
    if (f >= Y_GRID * dY) f = Y_GRID * dY-1;
    if (f < 0) f = 0;
    double f2 = ((data[x+1] / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
    f2 = (f2 - settingMin) * Y_GRID * dY / delta;
    if (f2 >= Y_GRID * dY) f2 = Y_GRID * dY-1;
    if (f2 < 0) f2 = 0;
  int Y1 = Y_GRID * dY - 1 - (int)f;
  int Y2 = Y_GRID * dY - 1 - (int)f2;
  DrawDirty(x,min(Y2,Y1));
  DrawDirty(x+1,min(Y2,Y1));
  tft.drawLine(x+oX, oY+Y1, x+oX+1, oY+Y2, color);
//  tft.drawLine(x+oX, oY+Y1+1, x+oX+1, oY+Y2, DISPLAY_YELLOW);
  sendDisplay();
}

void DisplayPeakData(void)
{
    double f = ((((float)myData[peakIndex]) / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
    int delta=settingMax - settingMin;
    f = (f - settingMin) * Y_GRID * dY / delta;
    if (f >= Y_GRID * dY) f = Y_GRID * dY-1;
    if (f < 0) f = 0;
  int Y1 = Y_GRID * dY - 1 - (int)f;
    tft.setCursor(oX+peakIndex+5,oY+Y1);             // Start at top-left corner
    tft.setTextColor(DISPLAY_WHITE);        // Draw white text
    tft.print(peakFreq/ 1000000.0);
    tft.setCursor(oX+peakIndex+5,oY+Y1+8);             // Start at top-left corner
    tft.print((int)((peakLevel/ 2.0  - settingAttenuate) - 120.0)+settingLevelOffset);
    tft.print("dB");
    for (int x=peakIndex+5;x<peakIndex+5+6*8;x++)
      DrawDirty(x,Y1);
}

#endif

void setup() 
{

  config.touch_cal[0] = 3841;
  config.touch_cal[1] = 3710;
  config.touch_cal[2] = -168;
  config.touch_cal[3] = -228;

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
#if USE_ILI9341 || USE_ILI9488
  tft.begin();
  tft.setRotation(1);
  ts.begin();
  ts.setRotation(1);
clearDisplay();
#endif
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
//  sendDisplay();
//  displayHisto();
  PE4302_init();
  PE4302_Write_Byte(0);
#ifdef USE_SI4463
  Si446x_init(); 
  Si446x_RX ((uint8_t)70);
#endif
// touch_cal_exec();
//touch_draw_test();
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

void SetRX(int p)
{
  RX = p;
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

int autoSweepStep = 0;
long autoSweepFreq = 0;
long autoSweepFreqStep = 0;
int standalone = true;

char test[258];

void loop()
{
/*
  int i;
  for (i=0;i<20;i++)
    test[i] = 140+i;
  test[20] = 0;
  test[0] = 1;
  ili9341_drawstring_size(test, 0,0, 0xffff, 0x0000, 2);
  ili9341_drawstring_size("01234567891123456789", 0,12, 0xffff, 0x0000, 2);
  return;
*/
  ui_process_touch();
  if (ui_mode != UI_NORMAL) {
    autoSweepStep = 0;
    return;
  }

  if (standalone) {
//    Serial.print("AutoSweepStep: ");
//    Serial.println(autoSweepStep);
   if (autoSweepStep == 0) {
      ownrbw = settingBandwidth;
      if (ownrbw == 0)
        ownrbw = 1.2*((float)(lFreq[1] - lFreq[0]))/DISPLAY_POINTS/1000.0;

      if (ownrbw < 2.6)
         ownrbw = 2.6;
      autoSweepFreq = lFreq[0];
      autoSweepFreqStep = (lFreq[1] - lFreq[0])/DISPLAY_POINTS;
      vbw = autoSweepFreqStep/1000.0;
      setFreq (0, lFreq[2]);
      lastFreq[0] = lFreq[2];
      startFreq = lFreq[0] + lFreq[2];
      stopFreq = lFreq[1] + lFreq[2];
      int p = - settingAttenuate * 2;
      PE4302_Write_Byte(p);
      SetPowerReference(settingPowerCal);
      SI4432_Sel = 0;
      ownrbw = SI4432_SET_RBW(ownrbw);
      SI4432_Sel = 1;
      SI4432_Write_Byte(0x6D, 0x1C + (drive - 2 )/2);//Set full power
      SetRX(settingGenerate ? 3 : 0);
      peakLevel = -150;
      peakFreq = -1.0;
      SI4432_Sel=1;
      setFreq (1, lFreq[2] + autoSweepFreq + (long)(ownrbw < 300.0?settingSpur * ownrbw:0));
    }
    if (autoSweepFreqStep >0 && autoSweepStep > 0) {
      SI4432_Sel=1;
      setFreq (1, lFreq[2] + autoSweepFreq + (long)(ownrbw < 300.0?settingSpur * ownrbw:0));
    }
    SI4432_Sel=0;
    int RSSI = SI4432_RSSI();
    if (vbw > ownrbw) {
      int subSteps = ((int)(1.5 * vbw / ownrbw)) - 1;
      
      while (subSteps > 0) {
//Serial.print("substeps = ");
//Serial.println(subSteps);
       SI4432_Sel=1;
       setFreq (1, lFreq[2] + autoSweepFreq + subSteps * ownrbw * 1000 + (long)(ownrbw < 300.0?settingSpur * ownrbw * 1000:0));
//Serial.print("Freq = ");
//Serial.println(lFreq[2] + autoSweepFreq + subSteps * ownrbw * 1000 + (long)(ownrbw < 300.0?settingSpur * ownrbw * 1000:0));
       SI4432_Sel=0;
       int subRSSI = SI4432_RSSI();
       if (RSSI < subRSSI)
         RSSI = subRSSI;
        subSteps--;
      }
    }
    DrawCheckerBoard(autoSweepStep);
// Debug AGC
    SI4432_Sel = 0;
     myStorage[autoSweepStep] = (SI4432_Read_Byte(0x69) & 0x01F) * 4;
// end debug
    if (settingShowStorage)
      DisplayPoint(myStorage, autoSweepStep, DISPLAY_BLUE);
      if (settingSubtractStorage)
        RSSI = 128 + RSSI - myStorage[autoSweepStep] ;
    if (dirty || settingAverage == AV_OFF)
      myData[autoSweepStep] = (unsigned char) RSSI;
    else {
      switch(settingAverage) {
      case AV_MIN: if (myData[autoSweepStep] > (unsigned char) RSSI) myData[autoSweepStep] = (unsigned char) RSSI; break;
      case AV_MAX: if (myData[autoSweepStep] < (unsigned char) RSSI) myData[autoSweepStep] = (unsigned char) RSSI; break;
      case AV_2: myData[autoSweepStep] = (myData[autoSweepStep] + RSSI) / 2; break;
      case AV_4: myData[autoSweepStep] = (myData[autoSweepStep]*3 + RSSI) / 4; break;
      case AV_8: myData[autoSweepStep] = (myData[autoSweepStep]*7 + RSSI) / 8; break;
      }
      myActual[autoSweepStep] = RSSI;
      DisplayPoint(myActual, autoSweepStep, DISPLAY_RED);
    }
    DisplayPoint(myData, autoSweepStep, DISPLAY_YELLOW);

    if (autoSweepFreq > 1000000) {
      if (peakLevel < myData[autoSweepStep]) {
        peakIndex = autoSweepStep;
        peakLevel = myData[autoSweepStep];
        peakFreq = autoSweepFreq;
      }
    }

    if (myData[autoSweepStep] == 0) {
        SI4432_Init();
    }
    if (!settingGenerate || autoSweepStep == 0) {
      autoSweepStep++;
      autoSweepFreq += (lFreq[1] - lFreq[0])/DISPLAY_POINTS;
    }
    if (autoSweepStep >= DISPLAY_POINTS) {
     // DisplayPeakData(); Does flikker
      if (settingAverage && dirty)
        dirty = false;
      autoSweepStep = 0;
      settingSpur = -settingSpur;
#if USE_DISPLAY
      displayHisto();
#endif
    }  
   }


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
    standalone = true;
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
        SetRX(lastParameter[6]);
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
