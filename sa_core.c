
// ---------------------------------------------------
#include "sa_core.h"
#include "SI4432.h"		// comment out for simulation

#ifndef __SI4432_H__
//-----------------SI4432 dummy------------------
void SI4432_Write_Byte(unsigned char ADR, unsigned char DATA ) {}
unsigned char SI4432_Read_Byte(unsigned char ADR) {return ADR;}
float SI4432_SET_RBW(float WISH) {return (WISH > 600.0?600: (WISH<3.0?3:WISH));}
void SI4432_SetPowerReference(int p) {}
void SI4432_Set_Frequency(long f) {}
void PE4302_Write_Byte(unsigned char DATA ) {}
void PE4302_init(void) {}

unsigned long seed = 123456789;
extern float rbw;
float myfrand(void)
{
  seed = (unsigned int) (1103515245 * seed + 12345) ;
  return ((float) seed) / 1000000000.0;
}
#define NOISE  ((myfrand()-2) * 2)  // +/- 4 dBm noise
extern int settingAttenuate;

//#define LEVEL(i, f, v) (v * (1-(fabs(f - frequencies[i])/rbw/1000)))

float LEVEL(uint32_t i, uint32_t f, int v)
{
  float dv;
  float df = fabs((float)f - (float)i);
  if (df < rbw*1000)
    dv = df/(rbw*1000);
  else
    dv =  1 + 50*(df - rbw*1000)/(rbw*1000);
  return (v - dv - settingAttenuate);
}

float SI4432_RSSI(uint32_t i, int s)
{
  SI4432_Sel = s;
  float v = -100 + log10(rbw)*10 + NOISE;
  if(s == 0) {
    v = fmax(LEVEL(i,10000000,-20),v);
    v = fmax(LEVEL(i,20000000,-40),v);
    v = fmax(LEVEL(i,30000000,-30),v);
    v = fmax(LEVEL(i,40000000,-90),v);
  } else {
    v = fmax(LEVEL(i,320000000,-20),v);
    v = fmax(LEVEL(i,340000000,-40),v);
    v = fmax(LEVEL(i,360000000,-30),v);
    v = fmax(LEVEL(i,380000000,-90),v);
  }
  return(v);
}
void SI4432_Init(void) {}
#endif
//--------------------- Frequency control -----------------------

int dirty = true;
int scandirty = true;

//---------------- menu system -----------------------

int settingAttenuate = 0;
int settingGenerate = 0;
int settingBandwidth = 0;
int settingLevelOffset = 0;
int settingPowerCal = 1;
int settingSpur = 0;
int settingAverage = 0;
int settingShowStorage = 0;
int settingSubtractStorage = 0;
int settingMode = 0;
int settingDrive=2; // 0-3 , 3=+20dBm

uint32_t minFreq = 0;
uint32_t maxFreq = 350000000;

void set_refer_output(int v)
{
  settingPowerCal = v;
  dirty = true;
}

void SetGenerate(int g)
{
  settingGenerate = g;
  dirty = true;
}

void SetMode(int m)
{
  settingMode = m;
  switch(m) {
  case M_LOW:
  case M_GENLOW:
    minFreq = 0;
    maxFreq = 350000000;
    set_sweep_frequency(ST_START, (int32_t) 0);
    set_sweep_frequency(ST_STOP, (int32_t) 300000000);
    break;
  case M_HIGH:
  case M_GENHIGH:
    minFreq = 260000000;
    maxFreq = 960000000;
    set_sweep_frequency(ST_START, (int32_t) 300000000);
    set_sweep_frequency(ST_STOP, (int32_t) 960000000);
    break;
  }
  dirty = true;
}


void SetAttenuation(int a)
{
  settingAttenuate = a;
  dirty = true;
}

void SetStorage(void)
{
  for (int i=0; i<POINT_COUNT;i++)
    stored_t[i] = actual_t[i];
  settingShowStorage = true;
  trace[TRACE_STORED].enabled = true;
}

void SetClearStorage(void)
{
  settingShowStorage = false;
  settingSubtractStorage = false;
  trace[TRACE_STORED].enabled = false;
  dirty = true;
}

void SetSubtractStorage(void)
{
  if (!settingShowStorage)
    SetStorage();
  settingSubtractStorage = true;
  dirty = true;
}

extern float peakLevel;
void SetPowerLevel(int o)
{
  if (o != 100)
    settingLevelOffset = o - peakLevel - settingAttenuate;
  else
    settingLevelOffset = 0;
  dirty = true;
}

void SetRBW(int v)
{
  settingBandwidth = v;
  update_rbw(frequencies[1] - frequencies[0]);
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
  trace[TRACE_TEMP].enabled = (v != 0);
  dirty = true;
}


//------------------------------------------


float peakLevel;
uint32_t peakFreq;
int peakIndex;
float temppeakLevel;
int temppeakIndex;

#define BARSTART  24


float rbw = 0;
float vbw = 0;
int vbwSteps = 1;

#if 0
int inData = 0;
unsigned long  startFreq = 250000000;
unsigned long  stopFreq = 300000000;
unsigned long  lastFreq[6] = { 300000000, 300000000,0,0,0,0};
int lastParameter[10];
int parameter;
unsigned long reg = 0;
long offset=0;
long offset2=0;
static unsigned int spacing = 10000;
double delta=0.0;
int phase=0;
int deltaPhase;
int delaytime = 50;
#endif


#if 0
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
    old_rbw = -1;
  }  

  if (old_rbw != rbw || old_vbw != vbw) {
    tft.fillRect(56, 0, 99, oY-2, DISPLAY_BLACK);
    tft.setCursor(56,0);             // Start at top-left corner
    tft.setTextColor(DISPLAY_WHITE);        // Draw white text
    tft.print("RBW:");
    tft.print(rbw);
    tft.print("kHz");
    tft.setCursor(56,8);             // Start at top-left corner
    tft.print("VBW:");
    tft.print(vbw);
    tft.print("kHz");
    old_rbw = rbw;
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
    double f = ((actual_t[i] / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
    f = (f - settingMin) * Y_GRID * dY / delta;
    if (f >= Y_GRID * dY) f = Y_GRID * dY-1;
    if (f < 0) f = 0;
    double f2 = ((actual_t[i+1] / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
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
  double f = ((((float)actual_t[peakIndex]) / 2.0  - settingAttenuate) - 120.0) + settingLevelOffset;
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

void setupSA(void)
{
  SI4432_Init();
  PE4302_init();
  PE4302_Write_Byte(0);
}


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

void SetRX(int m)
{
switch(m) {
case M_LOW:     // Mixed into 0
    SI4432_Sel = 0;
    SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain
    SI4432_Sel = 1;
    SI4432_Write_Byte(0x7, 0x0B); // start TX
    SI4432_Write_Byte(0x6D, 0x1C + settingDrive);//Set full power
    // SI4432_SetPowerReference(settingPowerCal);
    break;
case M_HIGH:    // Direct into 1
    // SI4432_SetPowerReference(-1); // Stop reference output
    SI4432_Sel = 0; // both as receiver to avoid spurs
    SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain
    SI4432_Sel = 1;
    SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain
    break;
case M_GENLOW:  // Mixed output from 0
    SI4432_Sel = 0;
    SI4432_Write_Byte(0x7, 0x0B); // start TX
    SI4432_Write_Byte(0x6D, 0x1C);//Set low power
    SI4432_Sel = 1;
    SI4432_Write_Byte(0x7, 0x0B); // start TX
    SI4432_Write_Byte(0x6D, 0x1C + settingDrive);//Set full power
    break;
case M_GENHIGH: // Direct output from 1
    SI4432_Sel = 0;
    SI4432_Write_Byte(0x7, 0x07); // set receive mode
    SI4432_Sel = 1;
    SI4432_Write_Byte(0x7, 0x0B); // start TX
    SI4432_Write_Byte(0x6D, 0x1C + settingDrive);//Set full power
    break;
  }
}

void update_rbw(uint32_t delta_f)
{
  vbw = (delta_f)/1000.0;
  rbw = settingBandwidth;
//  float old_rbw = rbw;
  if (rbw == 0)
    rbw = 1.2*vbw;
  if (rbw < 2.6)
    rbw = 2.6;
  if (rbw > 600)
    rbw = 600;
  SI4432_Sel = (settingMode & 1);
  rbw = SI4432_SET_RBW(rbw);
  vbwSteps = ((int)(1.5 * vbw / rbw)) - 1;
  if (vbwSteps < 1)
    vbwSteps = 1;
  dirty = true;
}

float perform(int i, int32_t f, int e)
{
  long local_IF = (settingMode == 0?frequency_IF:0);
  if (i == 0) {
    int p = settingAttenuate * 2;
    PE4302_Write_Byte(p);
    SetRX(settingMode);
    SI4432_SetPowerReference(settingPowerCal);
    temppeakLevel = -150;
    if (e == 0)
      setFreq (0, local_IF+f);
    else
      setFreq (0, local_IF);
    setFreq (1, local_IF + f + (long)(rbw < 300.0?settingSpur * rbw:0));
    if (dirty) {
      scandirty = true;
      dirty = false;
    }
  }
  uint32_t lf = f + (long)(rbw < 300.0?settingSpur * rbw:0);
  if (vbw >0 && i > 0) {
    if (e == 0)
      setFreq (0, local_IF+lf);
    setFreq (1, local_IF + lf);
  }
  float RSSI = SI4432_RSSI(lf, (settingMode & 1))+settingLevelOffset+settingAttenuate;
  if (vbw > rbw) {
    int subSteps = ((int)(1.5 * vbw / rbw)) - 1;

    while (subSteps > 0) {
      //Serial.print("substeps = ");
      //Serial.println(subSteps);
      lf = f + subSteps * rbw * 1000 + (long)(rbw < 300.0?settingSpur * rbw * 1000:0);
      if (e == 0)
        setFreq (0, local_IF+lf);
      setFreq (1, local_IF + lf);
      float subRSSI = SI4432_RSSI(lf, (settingMode & 1))+settingLevelOffset+settingAttenuate;
      if (RSSI < subRSSI)
        RSSI = subRSSI;
      subSteps--;
    }
  }
  if (RSSI == -120) {
//    SI4432_Init();
  }
  return(RSSI);
#if 0
  temp_t[i] = RSSI;
  if (settingSubtractStorage) {
    RSSI = RSSI - stored_t[i] ;
  }
  if (scandirty || settingAverage == AV_OFF)
    actual_t[i] = RSSI;
  else {
    switch(settingAverage) {
    case AV_MIN: if (actual_t[i] > RSSI) actual_t[i] = RSSI; break;
    case AV_MAX: if (actual_t[i] < RSSI) actual_t[i] = RSSI; break;
    case AV_2: actual_t[i] = (actual_t[i] + RSSI) / 2.0; break;
    case AV_4: actual_t[i] = (actual_t[i]*3 + RSSI) / 4.0; break;
    case AV_8: actual_t[i] = (actual_t[i]*7 + RSSI) / 8.0; break;
    }
  }
  if (frequencies[i] > 1000000) {
    if (temppeakLevel < actual_t[i]) {
      temppeakIndex = i;
      temppeakLevel = actual_t[i];
    }
  }
  if (temp_t[i] == 0) {
    SI4432_Init();
  }
  if (i == POINT_COUNT -1) {
    if (scandirty) {
      scandirty = false;
    }
    peakIndex = temppeakIndex;
    peakLevel = actual_t[peakIndex];
    peakFreq = frequencies[peakIndex];
    settingSpur = -settingSpur;
    int peak_marker = 0;
    markers[peak_marker].enabled = true;
    markers[peak_marker].index = peakIndex;
    markers[peak_marker].frequency = frequencies[markers[peak_marker].index];
//    redraw_marker(peak_marker, FALSE);


  }
#endif
}

// main loop for measurement
static bool sweep(bool break_on_operation)
{
  for (int i = 0; i < sweep_points; i++) {
    float RSSI = perform(i, frequencies[i], -1);
    temp_t[i] = RSSI;
    if (settingSubtractStorage) {
      RSSI = RSSI - stored_t[i] ;
    }
    if (scandirty || settingAverage == AV_OFF)
      actual_t[i] = RSSI;
    else {
      switch(settingAverage) {
      case AV_MIN: if (actual_t[i] > RSSI) actual_t[i] = RSSI; break;
      case AV_MAX: if (actual_t[i] < RSSI) actual_t[i] = RSSI; break;
      case AV_2: actual_t[i] = (actual_t[i] + RSSI) / 2.0; break;
      case AV_4: actual_t[i] = (actual_t[i]*3 + RSSI) / 4.0; break;
      case AV_8: actual_t[i] = (actual_t[i]*7 + RSSI) / 8.0; break;
      }
    }
    if (frequencies[i] > 1000000) {
      if (temppeakLevel < actual_t[i]) {
        temppeakIndex = i;
        temppeakLevel = actual_t[i];
      }
    }
    if (i == sweep_points -1) {
      if (scandirty) {
        scandirty = false;
      }
      peakIndex = temppeakIndex;
      peakLevel = actual_t[peakIndex];
      peakFreq = frequencies[peakIndex];
      settingSpur = -settingSpur;
      int peak_marker = 0;
      markers[peak_marker].enabled = true;
      markers[peak_marker].index = peakIndex;
      markers[peak_marker].frequency = frequencies[markers[peak_marker].index];
      //    redraw_marker(peak_marker, FALSE);


    }
    // back to toplevel to handle ui operation
    if (operation_requested && break_on_operation)
      return false;
  }
  return true;
}


#if 0
void PeakSearch()
{
#define PEAKSTACK   4
#define PEAKDISTANCE    10
  int level = 0;
  int searchLeft[PEAKSTACK];
  int peakIndex[PEAKSTACK];
  int peak_marker = 0;
  searchLeft[level] = true;
  peakIndex[level] = markers[peak_marker].index;
  level++;
  searchLeft[level] = true;
  int peakFrom;
  int peakTo;
  while (peak_marker < 4){
    if (searchLeft[level])
    {
      int fromLevel = level;
      while (fromLevel > 0 && searchLeft[fromLevel])
        fromLevel--
      if(fromLevel == 0) {
        peakFrom = PEAKDISTANCE;
      } else {
        peakFrom = peakIndex[fromLevel] + PEAKDISTANCE;
      }
      peakTo = peakIndex[level] - PEAKDISTANCE;
    } else {
      int toLevel = level;
      while (toLevel > 0 && !searchLeft[toLevel])
        toLevel--
      if(toLevel == 0) {
        peakTo = POINT_COUNT - 1 - PEAKDISTANCE;
      } else {
        peakTo = peakIndex[fromLevel] - PEAKDISTANCE;
      }
      peakFrom = peakIndex[level] + PEAKDISTANCE;
    }
    float peakMax = actual_t[peakFrom];
    int peakIndex = peakFrom;
    for (int i = peakFrom; i < peakTo; i++) {
      if (peakMax < actual_t[i]) {
        peakMax = actual_t[i];
        peakIndex = i;
      }
    }


  peakIndex = temppeakIndex;
  peakLevel = actual_t[peakIndex];
  peakFreq = frequencies[peakIndex];
  settingSpur = -settingSpur;
  int peak_marker = 0;
  markers[peak_marker].enabled = true;
  markers[peak_marker].index = peakIndex;
  markers[peak_marker].frequency = frequencies[markers[peak_marker].index];
//    redraw_marker(peak_marker, FALSE);


}

}
#endif

char *averageText[] = { "OFF", "MIN", "MAX", "2", "4", "8"};
char *dBText[] = { "1dB/", "2dB/", "5dB/", "10dB/", "20dB/"};

void draw_cal_status(void)
{
#define BLEN    10
  char buf[BLEN];
#define YSTEP   8
  int x = 0;
  int y = OFFSETY;

#define XSTEP   40

  ili9341_fill(x, y, OFFSETX, HEIGHT, 0x0000);

  int yMax = (YGRIDS - get_trace_refpos(0)) * get_trace_scale(0);
  chsnprintf(buf, BLEN, "%ddB", yMax);
  buf[5]=0;
  ili9341_drawstring_5x7(buf, x, y, 0xffff, 0x0000);

  y += YSTEP*2;
  chsnprintf(buf, BLEN, "%ddB/",(int)get_trace_scale(0));
  ili9341_drawstring_5x7(buf, x, y, 0xffff, 0x0000);

  y += YSTEP*2;
  ili9341_drawstring_5x7("Attn:", x, y, 0xffff, 0x0000);

  y += YSTEP;
  chsnprintf(buf, BLEN, "-%ddB", settingAttenuate);
  buf[5]=0;
  ili9341_drawstring_5x7(buf, x, y, 0xffff, 0x0000);


  if (settingAverage>0) {
    y += YSTEP*2;
    ili9341_drawstring_5x7("Aver:", x, y, 0xffff, 0x0000);

    y += YSTEP;
    chsnprintf(buf, BLEN, "%s",averageText[settingAverage]);
    buf[5]=0;
    ili9341_drawstring_5x7(buf, x, y, 0xffff, 0x0000);
  }

  if (settingSpur) {
    y += YSTEP*2;
    ili9341_drawstring_5x7("Spur:", x, y, 0xffff, 0x0000);

    y += YSTEP;
    chsnprintf(buf, BLEN, "ON");
    ili9341_drawstring_5x7(buf, x, y, 0xffff, 0x0000);
  }

  y += YSTEP*2;
  ili9341_drawstring_5x7("RBW:", x, y, 0xffff, 0x0000);

  y += YSTEP;
  chsnprintf(buf, BLEN, "%dkHz", (int)rbw);
  buf[5]=0;
  ili9341_drawstring_5x7(buf, x, y, 0xffff, 0x0000);

  y += YSTEP*2;
  ili9341_drawstring_5x7("VBW:", x, y, 0xffff, 0x0000);

  y += YSTEP;
  chsnprintf(buf, BLEN, "%dkHz",(int)vbw);
  buf[5]=0;
  ili9341_drawstring_5x7(buf, x, y, 0xffff, 0x0000);

  y += YSTEP*2;
  ili9341_drawstring_5x7("Scan:", x, y, 0xffff, 0x0000);

  y += YSTEP;
  chsnprintf(buf, BLEN, "%dS",(int)(0.005 * vbwSteps * sweep_points));
  buf[5]=0;
  ili9341_drawstring_5x7(buf, x, y, 0xffff, 0x0000);



  y = HEIGHT-7 + OFFSETY;
  chsnprintf(buf, BLEN, "%ddB", (int)(yMax - get_trace_scale(0) * YGRIDS));
  buf[5]=0;
  ili9341_drawstring_5x7(buf, x, y, 0xffff, 0x0000);
}



enum {
  TS_SIGNAL, TS_BELOW
};

enum {
  TS_SILENT, TS_10MHZ,
};

#define TEST_COUNT  4

static const struct {
  int kind;
  int setup;
  uint32_t center;
  int width;
  float pass;
  int stop_width;
  float stop;
} test_case [TEST_COUNT] =
{
 {TS_SIGNAL, TS_10MHZ, 10000000, 100000, -20, 30, -80 },
 {TS_SIGNAL, TS_10MHZ, 20000000, 100000, -40, 30, -90 },
 {TS_SIGNAL, TS_10MHZ, 30000000, 100000, -20, 30, -80 },
 {TS_BELOW,  TS_SILENT, 200000000, 100000000, -50, 0, 0},
};

enum {
  TS_WAITING, TS_PASS, TS_FAIL, TS_CRITICAL
};
static const  char *(test_text [4]) =
{
 "Waiting", "Pass", "Fail", "Critical"
};
static const  char *(test_fail_cause [TEST_COUNT]);

static int test_status[TEST_COUNT];
static int show_test_info = FALSE;
static volatile int test_wait = false;

static void test_acquire(int i)
{
  pause_sweep();
  set_sweep_frequency(ST_CENTER, (int32_t)test_case[i].center);
  set_sweep_frequency(ST_SPAN, (int32_t)test_case[i].width);

//  set_frequencies(test_case[i].center - test_case[i].width, test_case[i].center + test_case[i].width, POINT_COUNT);

//  sweep_once = TRUE;
//  while (sweep_once)
//    chThdSleepMilliseconds(10);
  //  chMtxUnlock(&mutex_sweep);
  sweep(false);

//  ui_process();

//  touch_start_watchdog();
//  draw_battery_status();
  plot_into_index(measured);
  redraw_request |= REDRAW_CELLS | REDRAW_FREQUENCY;
}

extern void cell_drawstring_5x7(int w, int h, char *str, int x, int y, uint16_t fg);

void cell_draw_test_info(int m, int n, int w, int h)
{
  char buf[35];
  if (!show_test_info)
    return;
  for (int i = -1; i < TEST_COUNT; i++) {
    int xpos = 25;
    int ypos = 40+i*8;
    xpos -= m * CELLWIDTH -CELLOFFSETX;
    ypos -= n * CELLHEIGHT;
    uint color;
    if (i < 0) {
      if (test_wait)
        chsnprintf(buf, sizeof buf, "Touch screen to continue");
      else
        buf[0] = 0;
      color = RGBHEX(0xFFFFFF);
    } else {
      chsnprintf(buf, sizeof buf, "Test %d: %s%s", i+1, test_fail_cause[i], test_text[test_status[i]] );
      if (test_status[i] == TS_PASS)
        color = RGBHEX(0x00FF00);
      else if (test_status[i] == TS_CRITICAL)
        color = RGBHEX(0xFFFF00);
      else if (test_status[i] == TS_FAIL)
        color = RGBHEX(0xFF7F7F);
      else
        color = RGBHEX(0x0000FF);
    }
    cell_drawstring_5x7(w, h, buf, xpos, ypos, color);
  }
}

#define fabs(X) ((X)<0?-(X):(X))

int validate_within(int i, float margin)
{
  if (fabs(peakLevel-test_case[i].pass) > margin)
    return false;
  return(test_case[i].center - 100000 < peakFreq && peakFreq < test_case[i].center + 100000 );
}

int validate_below(int i, float margin) {
  return(test_case[i].pass - peakLevel > margin);
}

void test_validate(int i)
{
  if (test_case[i].kind == TS_SIGNAL) {
    if (validate_within(i, 5.0))
      test_status[i] = TS_PASS;
    else if (validate_within(i, 10.0))
      test_status[i] = TS_CRITICAL;
    else
      test_status[i] = TS_FAIL;
    if (test_status[i] != TS_PASS)
      test_fail_cause[i] = "Peak ";
    if (test_status[i] == TS_PASS) {
      for (int j = 0; j < POINT_COUNT/2 - test_case[i].stop_width; j++) {
        if (actual_t[j] > test_case[i].stop - 5)
          test_status[i] = TS_CRITICAL;
        else if (actual_t[j] > test_case[i].stop) {
          test_status[i] = TS_FAIL;
          break;
        }
      }
      for (int j = POINT_COUNT/2 + test_case[i].stop_width; j < POINT_COUNT; j++) {
        if (actual_t[j] > test_case[i].stop - 5)
          test_status[i] = TS_CRITICAL;
        else if (actual_t[j] > test_case[i].stop) {
          test_status[i] = TS_FAIL;
          break;
        }
      }
      if (test_status[i] != TS_PASS)
        test_fail_cause[i] = "Stopband ";
    }
  } else if (test_case[i].kind == TS_BELOW) {
    if (validate_below(i, 10.0))
      test_status[i] = TS_PASS;
    else if (validate_below(i, 5.0))
      test_status[i] = TS_CRITICAL;
    else
      test_status[i] = TS_FAIL;
    if (test_status[i] != TS_PASS)
      test_fail_cause[i] = "Spur ";
  }
  if (test_status[i] != TS_PASS || i == TEST_COUNT - 1)
    test_wait = true;
  draw_all(TRUE);
  resume_sweep();
}

extern void menu_autosettings_cb(int item);
extern void touch_wait_release(void);

void self_test(void)
{
  menu_autosettings_cb(0);
  for (int i=0; i < TEST_COUNT; i++) {
    test_status[i] = TS_WAITING;
    test_fail_cause[i] = "";
  }
  show_test_info = TRUE;
  for (int i=0; i < TEST_COUNT; i++) {
    switch(test_case[i].setup) {
    case TS_SILENT:
      set_refer_output(-1);
      break;
    case TS_10MHZ:
      set_refer_output(2);
      break;
    }
    test_acquire(i);
    test_validate(i);
    chThdSleepMilliseconds(1000);
    if (test_status[i] != TS_PASS) {
      touch_wait_release();
    }
  }
  touch_wait_release();
  //  chThdSleepMilliseconds(2000);
  show_test_info = FALSE;
  menu_autosettings_cb(0);
}
