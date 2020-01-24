#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define Serial SerialUSB

#include "ui.h"

#define FALSE 0
#define TRUE -1


#include <XPT2046_Touchscreen.h>
#define CS_PIN  A3

#define TS_MINX 350
#define TS_MINY 200
#define TS_MAXX 3950
#define TS_MAXY 3850

XPT2046_Touchscreen ts(CS_PIN);


/*
 * Copyright (c) 2014-2015, TAKAHASHI Tomohiro (TTRFTECH) edy555@gmail.com
 * All rights reserved.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * The software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */


typedef struct {
  int8_t digit; /* 0~5 */
  int8_t digit_mode;
  int8_t current_trace; /* 0..3 */
  uint32_t value; // for editing at numeric input area
  uint32_t previous_value;
} uistat_t;


int cal_status;
#define CALSTAT_LOAD (1<<0)
#define CALSTAT_OPEN (1<<1)
#define CALSTAT_SHORT (1<<2)
#define CALSTAT_THRU (1<<3)
#define CALSTAT_ISOLN (1<<4)
#define CALSTAT_ES (1<<5)
#define CALSTAT_ER (1<<6)
#define CALSTAT_ET (1<<7)
#define CALSTAT_ED CALSTAT_LOAD
#define CALSTAT_EX CALSTAT_ISOLN
#define CALSTAT_APPLY (1<<8)
#define CALSTAT_INTERPOLATED (1<<9)

uistat_t uistat = {
6,
0,
0,
0,
0
};

config_t config = {
  1234,
  0,
  0xffff,
  0x3fff,
  { 0x3fff },
  { 3840, 3663, -166, -215 },
  0,
  0,
  0,
  0
};


enum {
  TRC_LOGMAG, TRC_PHASE, TRC_DELAY, TRC_SMITH, TRC_POLAR, TRC_LINEAR, TRC_SWR, TRC_REAL, TRC_IMAG, TRC_R, TRC_X, TRC_OFF
};

// LOGMAG: SCALE, REFPOS, REFVAL
// PHASE: SCALE, REFPOS, REFVAL
// DELAY: SCALE, REFPOS, REFVAL
// SMITH: SCALE, <REFPOS>, <REFVAL>
// LINMAG: SCALE, REFPOS, REFVAL
// SWR: SCALE, REFPOS, REFVAL

// Electrical Delay
// Phase

typedef struct {
  uint8_t enabled;
  uint8_t type;
  uint8_t channel;
  uint8_t polar;
  float scale;
  float refpos;
} trace_t;

trace_t trace[TRACE_COUNT];

#define NO_EVENT          0
#define EVT_BUTTON_SINGLE_CLICK   0x01
#define EVT_BUTTON_DOUBLE_CLICK   0x02
#define EVT_BUTTON_DOWN_LONG    0x04
#define EVT_UP          0x10
#define EVT_DOWN        0x20
#define EVT_REPEAT        0x40

#define BUTTON_DOWN_LONG_TICKS    5000  /* 1sec */
#define BUTTON_DOUBLE_TICKS     5000   /* 500ms */
#define BUTTON_REPEAT_TICKS     1000   /* 100ms */
#define BUTTON_DEBOUNCE_TICKS   200

/* lever switch assignment */
#define BIT_UP1   3
#define BIT_PUSH  2
#define BIT_DOWN1 1

#define READ_PORT() palReadPort(GPIOA)
#define BUTTON_MASK (0x0f)

static uint16_t last_button = 0;
static uint32_t last_button_down_ticks;
static uint32_t last_button_repeat_ticks;
static int8_t inhibit_until_release = FALSE;

enum { OP_NONE = 0, OP_LEVER, OP_TOUCH, OP_FREQCHANGE };

uint8_t operation_requested = OP_NONE;
#define MARKER_COUNT 5
int8_t previous_marker = -1;

typedef struct {
  int8_t enabled;
  int16_t index;
  uint32_t frequency;
} marker_t;

marker_t markers[MARKER_COUNT];
int active_marker;

enum {
  KM_START, KM_STOP, KM_CENTER, KM_SPAN, KM_CW, KM_SCALE, KM_REFPOS, KM_EDELAY, KM_VELOCITY_FACTOR, KM_SCALEDELAY
};

uint8_t ui_mode = UI_NORMAL;
static uint8_t keypad_mode;
static int8_t selection = 0;

typedef void (*menuaction_cb_t)(int item);
//typedef struct menuitem_t menuitem_t;

typedef struct menuitem_t {
  uint8_t type;
  const char *label;
#if 0
  union {
    const menuaction_cb_t pFunc;
    const menuitem_t* pMenu;
  };
#else
  void *action;
#endif
} menuitem_t;

// type of menu item 
enum {
    MT_NONE,
    MT_BLANK,
    MT_SUBMENU,
    MT_CALLBACK,
    MT_CANCEL,
//    MT_CLOSE
};

#define S_LARROW  "<-"
#define MENUITEM_MENU(text, pmenu) { MT_SUBMENU, text, (void *)pmenu }
#define MENUITEM_FUNC(text, pfunc) { MT_CALLBACK, text,  (void *)pfunc }
//#define MENUITEM_CLOSE { .type=MT_CLOSE, .label="CLOSE", .pMenu=NULL }
#define MENUITEM_BACK { MT_CANCEL, "<- BACK", (void *)NULL }
#define MENUITEM_END { MT_NONE, "", (void *)NULL } /* sentinel */


static int8_t last_touch_status = FALSE;
static int16_t last_touch_x;
static int16_t last_touch_y;
//int16_t touch_cal[4] = { 1000, 1000, 10*16, 12*16 };
//int16_t touch_cal[4] = { 620, 600, 130, 180 };
#define EVT_TOUCH_NONE 0
#define EVT_TOUCH_DOWN 1
#define EVT_TOUCH_PRESSED 2
#define EVT_TOUCH_RELEASED 3

int awd_count;
//int touch_x, touch_y;

#define NUMINPUT_LEN 10

#define KP_CONTINUE 0
#define KP_DONE 1
#define KP_CANCEL 2

static char kp_buf[11];
static int8_t kp_index = 0;


static void ui_mode_normal(void);
static void ui_mode_menu(void);
static void ui_mode_numeric(int _keypad_mode);
static void ui_mode_keypad(int _keypad_mode);
static void draw_menu(void);
static void leave_ui_mode(void);
static void erase_menu_buttons(void);
static void ui_process_keypad(void);
//static void ui_process_numeric(void);

static void menu_push_submenu(const menuitem_t *submenu);
static void menu_move_back(void);
static void menu_calop_cb(int item);
static void menu_caldone_cb(int item);
static void menu_save_cb(int item);
static void menu_cal2_cb(int item);
static void menu_trace_cb(int item);
static void menu_format2_cb(int item);
static void menu_format_cb(int item);
static void menu_scale_cb(int item);
static void menu_channel_cb(int item);
static void menu_transform_window_cb(int item);
static void menu_transform_cb(int item);
static void menu_stimulus_cb(int item);
static void menu_marker_sel_cb(int item);
static void menu_marker_op_cb(int item);
static void menu_recall_cb(int item);
static void menu_dfu_cb(int item);
static void menu_config_cb(int item);

// ===[MENU DEFINITION]=========================================================
static const menuitem_t menu_calop[] = {
  MENUITEM_FUNC("OPEN",     menu_calop_cb),
  MENUITEM_FUNC("SHORT",    menu_calop_cb),
  MENUITEM_FUNC("LOAD",     menu_calop_cb),
  MENUITEM_FUNC("ISOLN",    menu_calop_cb),
  MENUITEM_FUNC("THRU",     menu_calop_cb),
  MENUITEM_FUNC("DONE",     menu_caldone_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_save[] = {
  MENUITEM_FUNC("SAVE 0",   menu_save_cb),
  MENUITEM_FUNC("SAVE 1",   menu_save_cb),
  MENUITEM_FUNC("SAVE 2",   menu_save_cb),
  MENUITEM_FUNC("SAVE 3",   menu_save_cb),
 #if !defined(ANTENNA_ANALYZER)
  MENUITEM_FUNC("SAVE 4",   menu_save_cb),
  #endif
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_cal[] = {
  MENUITEM_MENU("CALIBRATE",    menu_calop),
  MENUITEM_MENU("SAVE",         menu_save),
  MENUITEM_FUNC("RESET",        menu_cal2_cb),
  MENUITEM_FUNC("CORRECTION",   menu_cal2_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_trace[] = {
  MENUITEM_FUNC("TRACE 0",      menu_trace_cb),
  MENUITEM_FUNC("TRACE 1",      menu_trace_cb),
//   #if !defined(ANTENNA_ANALYZER)
  MENUITEM_FUNC("TRACE 2",      menu_trace_cb),
  MENUITEM_FUNC("TRACE 3",      menu_trace_cb),
//    #endif
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_format2[] = {
  MENUITEM_FUNC("POLAR",        menu_format2_cb),
  MENUITEM_FUNC("LINEAR",       menu_format2_cb),
  MENUITEM_FUNC("REAL",         menu_format2_cb),
  MENUITEM_FUNC("IMAG",         menu_format2_cb),
  MENUITEM_FUNC("RESISTANCE",   menu_format2_cb),
  MENUITEM_FUNC("REACTANCE",    menu_format2_cb),
  MENUITEM_BACK,
  MENUITEM_END
};
#define S_RARROW "->"
static const menuitem_t menu_format[] = {
  MENUITEM_FUNC("LOGMAG",       menu_format_cb),
  MENUITEM_FUNC("PHASE",        menu_format_cb),
  MENUITEM_FUNC("DELAY",        menu_format_cb),
  MENUITEM_FUNC("SMITH",        menu_format_cb),
  MENUITEM_FUNC("SWR",          menu_format_cb),
  MENUITEM_MENU(S_RARROW" MORE", menu_format2),  
  //MENUITEM_FUNC("LINEAR",     menu_format_cb),
  //MENUITEM_FUNC("SWR",        menu_format_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_scale[] = {
  MENUITEM_FUNC("SCALE/DIV",     menu_scale_cb),
  MENUITEM_FUNC("\2REF\0LEVEL",  menu_scale_cb),
  MENUITEM_FUNC("ATTENUATOR",    menu_scale_cb),
  MENUITEM_FUNC("RBW",           menu_scale_cb),
  MENUITEM_BACK,
  MENUITEM_END
};


static const menuitem_t menu_channel[] = {
  MENUITEM_FUNC("\2CH0\0REFLECT",   menu_channel_cb),
  MENUITEM_FUNC("\2CH1\0THROUGH",   menu_channel_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_transform_window[] = {
  MENUITEM_FUNC("MINIMUM",      menu_transform_window_cb),
  MENUITEM_FUNC("NORMAL",       menu_transform_window_cb),
  MENUITEM_FUNC("MAXIMUM",      menu_transform_window_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_transform[] = {
  MENUITEM_FUNC("\2TRANSF\0ON",      menu_transform_cb),
  MENUITEM_FUNC("\2LP\0IMPULSE",  menu_transform_cb),
  MENUITEM_FUNC("\2LP\0STEP",     menu_transform_cb),
  MENUITEM_FUNC("BANDPASS",             menu_transform_cb),
  MENUITEM_MENU("WINDOW",               menu_transform_window),
  MENUITEM_FUNC("\2VEL\0FACTOR",   menu_transform_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_display[] = {
  MENUITEM_MENU("TRACE", menu_trace),
  MENUITEM_MENU("FORMAT", menu_format),
  MENUITEM_MENU("SCALE", menu_scale),
  MENUITEM_MENU("CHANNEL", menu_channel),
  MENUITEM_MENU("TRANSFORM", menu_transform),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_stimulus[] = {
  MENUITEM_FUNC("START",            menu_stimulus_cb),
  MENUITEM_FUNC("STOP",             menu_stimulus_cb),
  MENUITEM_FUNC("CENTER",           menu_stimulus_cb),
  MENUITEM_FUNC("SPAN",             menu_stimulus_cb),
  MENUITEM_FUNC("CW FREQ",          menu_stimulus_cb),
  MENUITEM_FUNC("\2PAUSE\0SWEEP",   menu_stimulus_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_marker_sel[] = {
  MENUITEM_FUNC("MARKER 1",     menu_marker_sel_cb),
  MENUITEM_FUNC("MARKER 2",     menu_marker_sel_cb),
  MENUITEM_FUNC("MARKER 3",     menu_marker_sel_cb),
  MENUITEM_FUNC("MARKER 4",     menu_marker_sel_cb),
  MENUITEM_FUNC("ALL OFF",      menu_marker_sel_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_marker[] = {
  MENUITEM_MENU("\2SELECT\0MARKER",     menu_marker_sel),
  MENUITEM_FUNC(S_RARROW"START",        menu_marker_op_cb),
  MENUITEM_FUNC(S_RARROW"STOP",         menu_marker_op_cb),
  MENUITEM_FUNC(S_RARROW"CENTER",       menu_marker_op_cb),
  MENUITEM_FUNC(S_RARROW"SPAN",         menu_marker_op_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_recall[] = {
  MENUITEM_FUNC("RECALL 0",         menu_recall_cb),
  MENUITEM_FUNC("RECALL 1",         menu_recall_cb),
  MENUITEM_FUNC("RECALL 2",         menu_recall_cb),
  MENUITEM_FUNC("RECALL 3",         menu_recall_cb),
   #if !defined(ANTENNA_ANALYZER)
  MENUITEM_FUNC("RECALL 4",         menu_recall_cb),
   #endif
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_dfu[] = {
  MENUITEM_FUNC("ENTER DFU", menu_dfu_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_config[] = {
  MENUITEM_FUNC("TOUCH CAL",    menu_config_cb),
  MENUITEM_FUNC("TOUCH TEST",   menu_config_cb),
  MENUITEM_FUNC("SAVE",         menu_config_cb),
  MENUITEM_FUNC("VERSION",      menu_config_cb),
  MENUITEM_MENU(S_RARROW"DFU",  menu_dfu),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_top[] = {
  MENUITEM_MENU("FREQUENCY",  menu_stimulus),
  MENUITEM_MENU("SCALE", menu_scale),
  MENUITEM_MENU("CONFIG",    menu_config),
  MENUITEM_END,

  MENUITEM_MENU("DISPLAY",   menu_display),
  MENUITEM_MENU("MARKER",    menu_marker),
  MENUITEM_MENU("CAL",       menu_cal),
  MENUITEM_MENU("RECALL",    menu_recall),
 // MENUITEM_CLOSE,
};

#define MENU_STACK_DEPTH_MAX 4
static uint8_t menu_current_level = 0;
static const menuitem_t *menu_stack[MENU_STACK_DEPTH_MAX] = {
  menu_top, NULL, NULL, NULL
};

// ===[/MENU DEFINITION]========================================================


#define OFFSETX 15
#define OFFSETY 0
//#define WIDTH 291
//#define HEIGHT 233


//boolean wastouched = true;
static int touch_check(void);

static void touch_wait_release(void)
{
  int status;
  /* wait touch release */
  do {
    status = touch_check();
  } while(status != EVT_TOUCH_RELEASED);
}

static int touch_status(void)
{
  return ts.touched();
}

int  btn_wait_release(void)
{
  touch_wait_release();
  return(EVT_TOUCH_RELEASED);
}
static int touch_check(void)
{
  int stat = touch_status();
  if (stat) {
//    Serial.print("Touch status ");
//    Serial.println(stat);
  
    TS_Point p = ts.getPoint();
    int x = p.x;
    int y = p.y;
    if (touch_status()) {
      last_touch_x = x;
      last_touch_y = y;
    }
  }

  if (stat != last_touch_status) {
    last_touch_status = stat;
    if (stat) {
          return EVT_TOUCH_PRESSED;
    } else {
      return EVT_TOUCH_RELEASED;
    }
  } else {
    if (stat) {
      return EVT_TOUCH_DOWN;
    } else {
      return EVT_TOUCH_NONE;
    }
  }
}


void touch_start_watchdog()
{
  // dummy
}

extern Adafruit_ILI9341 tft;

#define ili9341_fill  tft.fillRect
#define ili9341_line  tft.drawLine

void  ili9341_drawstring_size(const char *t, int x,int y, int tc, int bc, int size)
{
  
Serial.println(t);

  tft.setCursor(x,y);
  tft.setTextColor(tc); 
  tft.setTextSize(size);
  tft.print(t);
  tft.setTextSize(1);
}

int NF20x22; // dummy

char kpf[25] = {//                                                                back        up
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '-', 'x', 'k', 'M', 'G', (char)27, char(24), 'd', '+', 'k', 'n', 'p'
};

void ili9341_drawfont(char c, int *dummy,int x, int y, int fg, int bg)
{
  char t[2] = {kpf[c],0};
// Serial.println(t);
  ili9341_drawstring_size( (const char *)t, x,y, fg, bg, 3);
}


void ili9341_drawstring_5x7(const char *t, int x,int y, int tc, int bc)
{
  tft.setCursor(x,y);
  tft.setTextColor(tc); 
  tft.setTextSize(1);
  tft.print(t);
  Serial.print(t);
  Serial.print(", ") ;
  Serial.print(x);
  Serial.print(", ");
  Serial.print(y);
  Serial.print(", ");
  Serial.print(tc);
  Serial.print(", ");
  Serial.println(bc);
}

void touch_cal_exec(void)
{
  int status;
  int x1, x2, y1, y2;
  

  ili9341_fill(0, 0, 320, 240, 0);
  ili9341_line(0, 0, 0, 32, 0xffff);
  ili9341_line(0, 0, 32, 0, 0xffff);
#if !defined(ANTENNA_ANALYZER)
  ili9341_drawstring_5x7("TOUCH UPPER LEFT", 10, 10, 0xffff, 0x0000);
#else
ili9341_drawstring_7x13("TOUCH UPPER LEFT", 10, 10, 0xffff, 0x0000);
 #endif

  do {
    status = touch_check();
  } while(status != EVT_TOUCH_RELEASED);
  x1 = last_touch_x;
  y1 = last_touch_y;

  ili9341_fill(0, 0, 320, 240, 0);
  ili9341_line(320-1, 240-1, 320-1, 240-32, 0xffff);
  ili9341_line(320-1, 240-1, 320-32, 240-1, 0xffff);
   #if !defined(ANTENNA_ANALYZER)
  ili9341_drawstring_5x7("TOUCH LOWER RIGHT", 230, 220, 0xffff, 0x0000);
  #else
ili9341_drawstring_7x13("TOUCH LOWER RIGHT", 196, 214, 0xffff, 0x0000);
 #endif
  do {
    status = touch_check();
  } while(status != EVT_TOUCH_RELEASED);
  x2 = last_touch_x;
  y2 = last_touch_y;

  config.touch_cal[0] = x1;
  config.touch_cal[1] = y1;
  config.touch_cal[2] = (x2 - x1) * 16 / 320;
  config.touch_cal[3] = (y2 - y1) * 16 / 240;

Serial.println(config.touch_cal[0]);
Serial.println(config.touch_cal[1]);
Serial.println(config.touch_cal[2]);
Serial.println(config.touch_cal[3]);

  //redraw_all();
//  touch_start_watchdog();
}

void touch_position(int *x, int *y)
{
  *x = (last_touch_x - config.touch_cal[0]) * 16 / config.touch_cal[2];
  *y = (last_touch_y - config.touch_cal[1]) * 16 / config.touch_cal[3];
  Serial.print("TP = ");
  Serial.print(*x);
  Serial.print(", ");
  Serial.println(*y);
  
}

void touch_draw_test(void)
{
  int status;
  int x0, y0;
  int x1, y1;

  ili9341_fill(0, 0, 320, 240, 0);
     #if !defined(ANTENNA_ANALYZER)
  ili9341_drawstring_5x7("TOUCH TEST: DRAG PANEL", OFFSETX, 233, 0xffff, 0x0000);
 #else
ili9341_drawstring_7x13("TOUCH TEST: DRAG PANEL", OFFSETX, 227, 0xffff, 0x0000);
 #endif
  do {
    status = touch_check();
  } while(status != EVT_TOUCH_PRESSED);
  touch_position(&x0, &y0);

  do {
    status = touch_check();
    touch_position(&x1, &y1);
    ili9341_line(x0, y0, x1, y1, 0xffff);
    x0 = x1;
    y0 = y1;
//    chThdSleepMilliseconds(50);
  } while(status != EVT_TOUCH_RELEASED);

  touch_start_watchdog();
}

extern void show_logo(void);

void
show_version(void)
{
  

  show_logo();

  while (true) {
    if (touch_check() == EVT_TOUCH_PRESSED)
      break;
  }

  touch_start_watchdog();
}

#define RGB(r,g,b)  ( (((g)&0x1c)<<11) | (((b)&0xf8)<<5) | ((r)&0xf8) | (((g)&0xe0)>>5) )
#define RGBHEX(hex) ( (((hex)&0x001c00)<<3) | (((hex)&0x0000f8)<<5) | (((hex)&0xf80000)>>16) | (((hex)&0x00e000)>>13) )

void
show_logo(void)
{
  int x = 15, y = 30;
  ili9341_fill(0, 0, 320, 240, 0);
#if !defined(ANTENNA_ANALYZER)
  ili9341_drawstring_size("tinySA", x+60, y, RGBHEX(0x0000FF), 0x0000, 4);
  y += 25;

  ili9341_drawstring_size("NANOVNA.COM", x+100, y += 10, 0xffff, 0x0000, 2);
  ili9341_drawstring_5x7("https://github.com/hugen79/NanoVNA-H", x, y += 20, 0xffff, 0x0000);
  ili9341_drawstring_5x7("Based on edy555 design", x, y += 10, 0xffff, 0x0000);
  ili9341_drawstring_5x7("2016-2019 Copyright @edy555", x, y += 10, 0xffff, 0x0000);
  ili9341_drawstring_5x7("Licensed under GPL. See: https://github.com/ttrftech/NanoVNA", x, y += 10, 0xffff, 0x0000);
  ili9341_drawstring_5x7("Version: 0.0", x, y += 10, 0xffff, 0x0000);
  ili9341_drawstring_5x7("Build Time: ", x, y += 10, 0xffff, 0x0000);
//  y += 5;
//  ili9341_drawstring_5x7("Kernel: " CH_KERNEL_VERSION, x, y += 10, 0xffff, 0x0000);
//  ili9341_drawstring_5x7("Architecture: " PORT_ARCHITECTURE_NAME " Core Variant: " PORT_CORE_VARIANT_NAME, x, y += 10, 0xffff, 0x0000);
//  ili9341_drawstring_5x7("Port Info: " PORT_INFO, x, y += 10, 0xffff, 0x0000);
//  ili9341_drawstring_5x7("Platform: " PLATFORM_NAME, x, y += 10, 0xffff, 0x0000);

#else
  ili9341_drawstring_size(BOARD_NAME, x+80, y, RGBHEX(0x0000FF), 0x0000, 2);
    y += 14;

    ili9341_drawstring_7x13("NANOVNA.COM", x+100, y += 15, 0xffff, 0x0000);
    ili9341_drawstring_7x13("https://github.com/hugen79/NanoVNA-H", x, y += 15, 0xffff, 0x0000);
    ili9341_drawstring_7x13("Based on edy555 design", x, y += 15, 0xffff, 0x0000);
    ili9341_drawstring_7x13("2016-2019 Copyright @edy555", x, y += 15, 0xffff, 0x0000);
    ili9341_drawstring_7x13("https://github.com/ttrftech/NanoVNA", x, y += 15, 0xffff, 0x0000);
    ili9341_drawstring_7x13("Version: " VERSION, x, y += 15, 0xffff, 0x0000);
    ili9341_drawstring_7x13("Build Time: " __DATE__ " - " __TIME__, x, y += 15, 0xffff, 0x0000);
#endif
}



void enter_dfu(void)
{
}

void draw_cal_status()
{
  
}

void redraw_frame() {}
void request_to_redraw_grid() { clearDisplay();}
void update_grid() {}
void draw_frequencies() {}


static void menu_calop_cb(int item)
{
  switch (item) {
  case 0: // OPEN
 //   cal_collect(CAL_OPEN);
    break;
  case 1: // SHORT
//    cal_collect(CAL_SHORT);
    break;
  case 2: // LOAD
//    cal_collect(CAL_LOAD);
    break;
  case 3: // ISOLN
//    cal_collect(CAL_ISOLN);
    break;
  case 4: // THRU
//    cal_collect(CAL_THRU);
    break;
  }
  selection = item+1;
  draw_cal_status();
  draw_menu();
}

static void menu_caldone_cb(int item)
{
  (void) item;
//  cal_done();
  draw_cal_status();
  menu_move_back();
  menu_push_submenu(menu_save);
}

static void menu_cal2_cb(int item)
{
  switch (item) {
  case 2: // RESET
    cal_status = 0;
    break;
  case 3: // CORRECTION
    // toggle applying correction
//    if (cal_status)
//      cal_status ^= CALSTAT_APPLY;
    draw_menu();
    break;
  }
  draw_cal_status();
  //menu_move_back();
}

static void menu_recall_cb(int item)
{
  if (item < 0 || item >= 5)
    return;
//  if (caldata_recall(item) == 0) 
  {
    menu_move_back();
    ui_mode_normal();
    update_grid();
    draw_cal_status();
  }
}


static void menu_config_cb(int item)
{
  switch (item) {
  case 0:
      touch_cal_exec();
      redraw_frame();
      request_to_redraw_grid();
      draw_menu();
      break;
  case 1:
      touch_draw_test();
      redraw_frame();
      request_to_redraw_grid();
      draw_menu();
      break;
  case 2:
 //     config_save();
      menu_move_back();
      ui_mode_normal();
      break;
  case 3:
      show_version();
      redraw_frame();
      request_to_redraw_grid();
      draw_menu();
  }
}

static void menu_dfu_cb(int item)
{
  switch (item) {
  case 0:
      enter_dfu();
  }
}

static void menu_save_cb(int item)
{
  if (item < 0 || item >= 5)
    return;
//  if (caldata_save(item) == 0) 
  {
    menu_move_back();
    ui_mode_normal();
    draw_cal_status();
  }
}

static void choose_active_trace(void)
{
  int i;
  if (trace[uistat.current_trace].enabled)
    // do nothing
    return;
  for (i = 0; i < TRACE_COUNT ; i++)
    if (trace[i].enabled) {
      uistat.current_trace = i;
      return;
    }
}

static void menu_trace_cb(int item)
{
  if (item < 0 || item >= TRACE_COUNT )
    return;
  if (trace[item].enabled) {
    if (item == uistat.current_trace) {
      // disable if active trace is selected
      trace[item].enabled = FALSE;
      choose_active_trace();
    } else {
      // make active selected trace
      uistat.current_trace = item;
    }
  } else {
    trace[item].enabled = TRUE;
    uistat.current_trace = item;
  }
  request_to_redraw_grid();
  draw_menu();
}

static void menu_format_cb(int item)
{
  switch (item) {
  case 0:
//    set_trace_type(uistat.current_trace, TRC_LOGMAG);
    break;
  case 1:
//    set_trace_type(uistat.current_trace, TRC_PHASE);
    break;
  case 2:
//    set_trace_type(uistat.current_trace, TRC_DELAY);
    break;
  case 3:
//    set_trace_type(uistat.current_trace, TRC_SMITH);
    break;
  case 4:
//    set_trace_type(uistat.current_trace, TRC_SWR);
    break;
  }

  request_to_redraw_grid();
  ui_mode_normal();
  //redraw_all();
}

static void menu_format2_cb(int item)
{
  switch (item) {
  case 0:
//    set_trace_type(uistat.current_trace, TRC_POLAR);
    break;
  case 1:
//    set_trace_type(uistat.current_trace, TRC_LINEAR);
    break;
  case 2:
//    set_trace_type(uistat.current_trace, TRC_REAL);
    break;
  case 3:
//    set_trace_type(uistat.current_trace, TRC_IMAG);
    break;
  case 4:
//    set_trace_type(uistat.current_trace, TRC_R);
    break;
  case 5:
//    set_trace_type(uistat.current_trace, TRC_X);
    break;
  }

  request_to_redraw_grid();
  ui_mode_normal();
}

static void menu_channel_cb(int item)
{
  if (item < 0 || item >= 2)
    return;
//  set_trace_channel(uistat.current_trace, item);
  menu_move_back();
  ui_mode_normal();
}

static void menu_transform_window_cb(int item)
{
  // TODO
  switch (item) {
    case 0:
 //     domain_mode = (domain_mode & ~TD_WINDOW) | TD_WINDOW_MINIMUM;
      ui_mode_normal();
      break;
    case 1:
//      domain_mode = (domain_mode & ~TD_WINDOW) | TD_WINDOW_NORMAL;
      ui_mode_normal();
      break;
    case 2:
//      domain_mode = (domain_mode & ~TD_WINDOW) | TD_WINDOW_MAXIMUM;
      ui_mode_normal();
      break;
  }
}

static void menu_transform_cb(int item)
{
  int status;
  switch (item) {
    case 0:
      draw_frequencies();
      ui_mode_normal();
      break;
    case 1:
      ui_mode_normal();
      break;
    case 2:
      ui_mode_normal();
      break;
    case 3:
      ui_mode_normal();
      break;
    case 5:
      status = btn_wait_release();
      if (status & EVT_BUTTON_DOWN_LONG) {
        ui_mode_numeric(KM_VELOCITY_FACTOR);
//        ui_process_numeric();
      } else {
        ui_mode_keypad(KM_VELOCITY_FACTOR);
        ui_process_keypad();
      }
      break;
  }
}

static void choose_active_marker(void)
{
  int i;
  for (i = 0; i < MARKER_COUNT; i++)
    if (markers[i].enabled) {
      active_marker = i;
      return;
    }
  active_marker = -1;
}

static void menu_scale_cb(int item)
{
  int status;
  int km = KM_SCALE + item;
//  if (km == KM_SCALE && trace[uistat.current_trace].type == TRC_DELAY) {
//    km = KM_SCALEDELAY;
//  }
  status = btn_wait_release();
  if (status & EVT_BUTTON_DOWN_LONG) {
    ui_mode_numeric(km);
//    ui_process_numeric();
  } else {
    ui_mode_keypad(km);
    ui_process_keypad();
  }
}

static void menu_stimulus_cb(int item)
{
  int status;
  switch (item) {
  case 0: /* START */
  case 1: /* STOP */
  case 2: /* CENTER */
  case 3: /* SPAN */
  case 4: /* CW */
    status = btn_wait_release();
    if (status & EVT_BUTTON_DOWN_LONG) {
      ui_mode_numeric(item);
//      ui_process_numeric();
    } else {
      ui_mode_keypad(item);
      ui_process_keypad();
    }
    break;
  case 5: /* PAUSE */
 //   toggle_sweep();
    //menu_move_back();
    //ui_mode_normal();
    draw_menu();
    break;
  }
}

int frequencies(int f)
{
  return(f); 
}

static int32_t get_marker_frequency(int marker)
{
  if (marker < 0 || marker >= MARKER_COUNT)
    return -1;
  if (!markers[marker].enabled)
    return -1;
  return frequencies(markers[marker].index);
}

static void menu_marker_op_cb(int item)
{
  int32_t freq = get_marker_frequency(active_marker);
  if (freq < 0)
    return; // no active marker

  switch (item) {
  case 1: /* MARKER->START */
//    set_sweep_frequency(ST_START, freq);
    break;
  case 2: /* MARKER->STOP */
//    set_sweep_frequency(ST_STOP, freq);
    break;
  case 3: /* MARKER->CENTER */
//    set_sweep_frequency(ST_CENTER, freq);
    break;
  case 4: /* MARKERS->SPAN */
    {
      if (previous_marker == active_marker)
        return;
      int32_t freq2 = get_marker_frequency(previous_marker);
      if (freq2 < 0)
        return;
      if (freq > freq2) {
        freq2 = freq;
        freq = get_marker_frequency(previous_marker);
      }
 //     set_sweep_frequency(ST_START, freq);
 //     set_sweep_frequency(ST_STOP, freq2);
#if 0
      int32_t span = (freq - freq2) * 2;
      if (span < 0) span = -span;
      set_sweep_frequency(ST_SPAN, span);
#endif
    }
    break;
  }
  ui_mode_normal();
  draw_cal_status();
  //redraw_all();
}

static void active_marker_select(int item)
{
  if (item == -1) {
    active_marker = previous_marker;
    previous_marker = -1;
    if (active_marker == -1) {
      choose_active_marker();
    }
  } else {
    if (previous_marker != active_marker)
      previous_marker = active_marker;
    active_marker = item;
  }
}

static void menu_marker_sel_cb(int item)
{
  if (item >= 0 && item < 4 && item < MARKER_COUNT) {
    if (markers[item].enabled) {
      if (item == active_marker) {
        // disable if active trace is selected
        markers[item].enabled = FALSE;
        active_marker_select(-1);
      } else {
        active_marker_select(item);
      }
    } else {
      markers[item].enabled = TRUE;
      active_marker_select(item);
    }
  } else if (item == 4) { /* all off */
      markers[0].enabled = FALSE;
      markers[1].enabled = FALSE;
      markers[2].enabled = FALSE;
      markers[3].enabled = FALSE;
      previous_marker = -1;
      active_marker = -1;      
  }
//  redraw_marker(active_marker, TRUE);
  draw_menu();
}

static void ensure_selection(void)
{
  const menuitem_t *menu = menu_stack[menu_current_level];
  int i;
  for (i = 0; menu[i].type != MT_NONE; i++)
    ;
  if (selection >= i)
    selection = i-1;
}

static void menu_move_back(void)
{
  if (menu_current_level == 0)
    return;

Serial.print("Poplevel=");
Serial.println(menu_current_level);

  menu_current_level--;
  ensure_selection();
  erase_menu_buttons();
  draw_menu();
}

static void menu_push_submenu(const menuitem_t *submenu)
{

Serial.print("Pushlevel=");
Serial.println(menu_current_level);

  if (menu_current_level < MENU_STACK_DEPTH_MAX-1)
    menu_current_level++;
  menu_stack[menu_current_level] = submenu;
  ensure_selection();
  erase_menu_buttons();
  draw_menu();
}

/*
static void menu_move_top(void)
{
  if (menu_current_level == 0)
    return;
  menu_current_level = 0;
  ensure_selection();
  erase_menu_buttons();
  draw_menu();
}
*/

static void menu_invoke(int item)
{
  const menuitem_t *menu = menu_stack[menu_current_level];
  menu = &menu[item];

Serial.print("Invoke=");
Serial.print(item);
Serial.print(", type=");
Serial.print(menu->type);
Serial.print(", label= ");
Serial.println(menu->label);


  switch (menu->type) {
  case MT_NONE:
  case MT_BLANK:
//  case MT_CLOSE:
    ui_mode_normal();
    break;

  case MT_CANCEL:
    menu_move_back();
    break;

  case MT_CALLBACK: {
    menuaction_cb_t cb = (menuaction_cb_t) menu->action;
    if (cb == NULL)
      return;
    (*cb)(item);
    break;
  }

  case MT_SUBMENU:
    menu_push_submenu((const menuitem_t *)menu->action);
    break;
  }
}
#if !defined(ANTENNA_ANALYZER)
#define KP_X(x) (48*(x) + 2 + (320-64-192))
#else
#define KP_X(x) (48*(x) + 2 + (320-72-192))
#endif
#define KP_Y(y) (48*(y) + 2)

#define KP_PERIOD 10
#define KP_MINUS 11
#define KP_X1 12
#define KP_K 13
#define KP_M 14
#define KP_G 15
#define KP_BS 16
#define KP_INF 17
#define KP_DB 18
#define KP_PLUSMINUS 19
#define KP_KEYPAD 20
#define KP_N 21
#define KP_P 22

typedef struct {
  uint16_t x, y;
  int8_t c;
} keypads_t;

static const keypads_t *keypads;
static uint8_t keypads_last_index;

static const keypads_t keypads_freq[] = {
  { KP_X(1), KP_Y(3), KP_PERIOD },
  { KP_X(0), KP_Y(3), 0 },
  { KP_X(0), KP_Y(2), 1 },
  { KP_X(1), KP_Y(2), 2 },
  { KP_X(2), KP_Y(2), 3 },
  { KP_X(0), KP_Y(1), 4 },
  { KP_X(1), KP_Y(1), 5 },
  { KP_X(2), KP_Y(1), 6 },
  { KP_X(0), KP_Y(0), 7 },
  { KP_X(1), KP_Y(0), 8 },
  { KP_X(2), KP_Y(0), 9 },
  { KP_X(3), KP_Y(0), KP_G },
  { KP_X(3), KP_Y(1), KP_M },
  { KP_X(3), KP_Y(2), KP_K },
  { KP_X(3), KP_Y(3), KP_X1 },
  { KP_X(2), KP_Y(3), KP_BS },
  { 0, 0, -1 }
};

static const keypads_t keypads_scale[] = {
  { KP_X(1), KP_Y(3), KP_PERIOD },
  { KP_X(0), KP_Y(3), 0 },
  { KP_X(0), KP_Y(2), 1 },
  { KP_X(1), KP_Y(2), 2 },
  { KP_X(2), KP_Y(2), 3 },
  { KP_X(0), KP_Y(1), 4 },
  { KP_X(1), KP_Y(1), 5 },
  { KP_X(2), KP_Y(1), 6 },
  { KP_X(0), KP_Y(0), 7 },
  { KP_X(1), KP_Y(0), 8 },
  { KP_X(2), KP_Y(0), 9 },
  { KP_X(3), KP_Y(3), KP_X1 },
  { KP_X(2), KP_Y(3), KP_BS },
  { 0, 0, -1 }
};

static const keypads_t keypads_time[] = {
  { KP_X(1), KP_Y(3), KP_PERIOD },
  { KP_X(0), KP_Y(3), 0 },
  { KP_X(0), KP_Y(2), 1 },
  { KP_X(1), KP_Y(2), 2 },
  { KP_X(2), KP_Y(2), 3 },
  { KP_X(0), KP_Y(1), 4 },
  { KP_X(1), KP_Y(1), 5 },
  { KP_X(2), KP_Y(1), 6 },
  { KP_X(0), KP_Y(0), 7 },
  { KP_X(1), KP_Y(0), 8 },
  { KP_X(2), KP_Y(0), 9 },
  { KP_X(3), KP_Y(1), KP_N },
  { KP_X(3), KP_Y(2), KP_P },
  { KP_X(3), KP_Y(3), KP_MINUS },  
  { KP_X(2), KP_Y(3), KP_BS },
  { 0, 0, -1 }
};

static const keypads_t * const keypads_mode_tbl[] = {
  keypads_freq, // start
  keypads_freq, // stop
  keypads_freq, // center
  keypads_freq, // span
  keypads_freq, // cw freq
  keypads_scale, // scale
  keypads_scale, // refpos
  keypads_time, // electrical delay
  keypads_scale, // velocity factor
  keypads_time // scale of delay
};

static const char * const keypad_mode_label[] = {
  "START", "STOP", "CENTER", "SPAN", "CW FREQ", "SCALE", "REFPOS", "EDELAY", "VELOCITY%", "DELAY"
};

static void draw_keypad(void)
{
  int i = 0;
  while (keypads[i].x) {
    uint16_t bg = config.menu_normal_color;
    if (i == selection)
      bg = config.menu_active_color;
    ili9341_fill(keypads[i].x, keypads[i].y, 44, 44, bg);
    ili9341_drawfont(keypads[i].c, &NF20x22, keypads[i].x+12, keypads[i].y+10, 0x0000, bg);
    i++;
  }
}

static void draw_numeric_area_frame(void)
{
  ili9341_fill(0, 208, 320, 32, 0xffff);
#if !defined(ANTENNA_ANALYZER)
  ili9341_drawstring_5x7(keypad_mode_label[keypad_mode], 10, 220, 0x0000, 0xffff);
#else
  ili9341_drawstring_7x13(keypad_mode_label[keypad_mode], 10, 220, 0x0000, 0xffff);
#endif
  //ili9341_drawfont(KP_KEYPAD, &NF20x22, 300, 216, 0x0000, 0xffff);
}

static void draw_numeric_input(const char *buf)
{
  int i = 0;
  int x = 64;
  int focused = FALSE;
  const uint16_t xsim[] = { 0, 0, 8, 0, 0, 8, 0, 0, 0, 0 };
  for (i = 0; i < 10 && buf[i]; i++) {
    uint16_t fg = 0x0000;
    uint16_t bg = 0xffff;
    int c = buf[i];
    if (c == '.')
      c = KP_PERIOD;
    else if (c == '-')
      c = KP_MINUS;
    else if (c >= '0' && c <= '9')
      c = c - '0';
    else
      c = -1;

    if (uistat.digit == 8-i) {
      fg = RGBHEX(0xf7131f);
      focused = TRUE;
      if (uistat.digit_mode)
        bg = 0x0000;
    }

    if (c >= 0)
      ili9341_drawfont(c, &NF20x22, x, 208+4, fg, bg);
    else if (focused)
      ili9341_drawfont(0, &NF20x22, x, 208+4, fg, bg);
    else
      ili9341_fill(x, 208+4, 20, 24, bg);
      
    x += 20;
    if (xsim[i] > 0) {
      //ili9341_fill(x, 208+4, xsim[i], 20, bg);
      x += xsim[i];
    }
  }
  if (i < 10) {
      ili9341_fill(x, 208+4, 20*(10-i), 24, 0xffff);
  }
}

static int menu_is_multiline(const char *label, const char **l1, const char **l2)
{
  if (label[0] != '\2')
    return FALSE;

  *l1 = &label[1];
  *l2 = &label[1] + strlen(&label[1]) + 1;
  return TRUE;
}

int sweep_enabled = 1;
static void menu_item_modify_attribute(
    const struct menuitem_t *menu, int item, uint16_t *fg, uint16_t *bg)
{
  if (menu == menu_trace && item < TRACE_COUNT  && item < MARKER_COUNT) {
    if (trace[item].enabled)
      *bg = config.trace_color[item];
  } else if (menu == menu_marker_sel && item < 4&& item < MARKER_COUNT) {
    if (markers[item].enabled) {
      *bg = 0x0000;
      *fg = 0xffff;
    }   
  } else if (menu == menu_calop) {
    if ((item == 0 && (cal_status & CALSTAT_OPEN))
        || (item == 1 && (cal_status & CALSTAT_SHORT))
        || (item == 2 && (cal_status & CALSTAT_LOAD))
        || (item == 3 && (cal_status & CALSTAT_ISOLN))
        || (item == 4 && (cal_status & CALSTAT_THRU))) {
 //     domain_mode = (domain_mode & ~DOMAIN_MODE) | DOMAIN_FREQ;
      *bg = 0x0000;
      *fg = 0xffff;
    }
  } else if (menu == menu_stimulus) {
    if (item == 5 /* PAUSE */ && !sweep_enabled) {
      *bg = 0x0000;
      *fg = 0xffff;
    }
  } else if (menu == menu_cal) {
    if (item == 3 /* CORRECTION */ && (cal_status & CALSTAT_APPLY)) {
      *bg = 0x0000;
      *fg = 0xffff;
    }
  } else if (menu == menu_transform) {
  } else if (menu == menu_transform_window) {
  }
}

static void draw_menu_buttons(const menuitem_t *menu)
{
  int i = 0;
Serial.println("----------------------------");
  for (i = 0; i < 7; i++) {
    const char *l1, *l2;
    if (menu[i].type == MT_NONE)
      break;
    if (menu[i].type == MT_BLANK) 
      continue;
    int y = 32*i;
    uint16_t bg = config.menu_normal_color;
    uint16_t fg = 0x0000;
    // focus only in MENU mode but not in KEYPAD mode
    if (ui_mode == UI_MENU && i == selection)
      bg = config.menu_active_color;
#if !defined(ANTENNA_ANALYZER)
    ili9341_fill(320-60, y, 60, 30, bg);
    
    menu_item_modify_attribute(menu, i, &fg, &bg);
    if (menu_is_multiline(menu[i].label, &l1, &l2)) {
      ili9341_drawstring_5x7(l1, 320-54, y+8, fg, bg);
      ili9341_drawstring_5x7(l2, 320-54, y+15, fg, bg);
    } else {
      ili9341_drawstring_5x7(menu[i].label, 320-54, y+12, fg, bg);
    }

#else
    ili9341_fill(320-72, y, 72, 30, bg);

        menu_item_modify_attribute(menu, i, &fg, &bg);
        if (menu_is_multiline(menu[i].label, &l1, &l2)) {
          ili9341_drawstring_7x13(l1, 320-70, y+3, fg, bg);
          ili9341_drawstring_7x13(l2, 320-70, y+16, fg, bg);
        } else {
          ili9341_drawstring_7x13(menu[i].label, 320-70, y+9, fg, bg);
        }
#endif
  }
}

static void menu_select_touch(int i)
{
  selection = i;
  draw_menu();
Serial.println("Before wait release");
  touch_wait_release();
Serial.println("After wait release");
  selection = -1;
  menu_invoke(i);
}

static void menu_apply_touch(void)
{
  int touch_x, touch_y;
  const menuitem_t *menu = menu_stack[menu_current_level];
  int i;

  touch_position(&touch_x, &touch_y);
  for (i = 0; i < 7; i++) {
    if (menu[i].type == MT_NONE)
      break;
    if (menu[i].type == MT_BLANK) 
      continue;
    int y = 32*i;
#if !defined(ANTENNA_ANALYZER)
    if (y-2 < touch_y && touch_y < y+30+2
    && 320-60 < touch_x) {
#else
  if (y-2 < touch_y && touch_y < y+30+2
    && 320-72 < touch_x) {
#endif
      menu_select_touch(i);
      return;
    }
  }

  touch_wait_release();
  ui_mode_normal();
}

static void draw_menu(void)
{
  
  Serial.println("draw menu");
  draw_menu_buttons(menu_stack[menu_current_level]);
}

static void erase_menu_buttons(void)
{
  uint16_t bg = 0;
  Serial.println("erase buttons");
  #if !defined(ANTENNA_ANALYZER)
  ili9341_fill(320-60, 0, 60, 32*7, bg);
  #else
   ili9341_fill(320-72, 0, 72, 32*7, bg);
#endif

}

static void erase_numeric_input(void)
{
  uint16_t bg = 0;
  ili9341_fill(0, 240-32, 320, 32, bg);
}
void request_to_draw_cells_behind_menu() {}

static void leave_ui_mode(void)
{
  if (ui_mode == UI_MENU) {
    request_to_draw_cells_behind_menu();
    erase_menu_buttons();
  } else if (ui_mode == UI_NUMERIC) {
//    request_to_draw_cells_behind_numeric_input();
    erase_numeric_input();
//    draw_frequencies();
  }
// Serial.println("leave UI");
}

static void fetch_numeric_target(void)
{
  switch (keypad_mode) {
  case KM_START:
   uistat.value = get_sweep_frequency(ST_START);
    break;
  case KM_STOP:
   uistat.value = get_sweep_frequency(ST_STOP);
    break;
  case KM_CENTER:
   uistat.value = get_sweep_frequency(ST_CENTER);
    break;
  case KM_SPAN:
    uistat.value = get_sweep_frequency(ST_SPAN);
    break;
  case KM_CW:
    uistat.value = get_sweep_frequency(ST_CW);
    break;
  case KM_SCALE:
//    uistat.value = get_trace_scale(uistat.current_trace) * 1000;
    break;
  case KM_REFPOS:
//    uistat.value = get_trace_refpos(uistat.current_trace) * 1000;
    break;
  case KM_EDELAY:
//    uistat.value = get_electrical_delay();
    break;
  case KM_VELOCITY_FACTOR:
//    uistat.value = velocity_factor;
    break;
  case KM_SCALEDELAY:
//    uistat.value = get_trace_scale(uistat.current_trace) * 1e12;
    break;
  }
  
  {
    uint32_t x = uistat.value;
    int n = 0;
    for (; x >= 10 && n < 9; n++)
      x /= 10;
    uistat.digit = n;
  }
  uistat.previous_value = uistat.value;
}

#if 0
static void set_numeric_value(void)
{
  switch (keypad_mode) {
  case KM_START:
    set_sweep_frequency(ST_START, uistat.value);
    break;
  case KM_STOP:
    set_sweep_frequency(ST_STOP, uistat.value);
    break;
  case KM_CENTER:
    set_sweep_frequency(ST_CENTER, uistat.value);
    break;
  case KM_SPAN:
    set_sweep_frequency(ST_SPAN, uistat.value);
    break;
  case KM_CW:
    set_sweep_frequency(ST_CW, uistat.value);
    break;
  case KM_SCALE:
    set_trace_scale(uistat.current_trace, uistat.value / 1000.0);
    break;
  case KM_REFPOS:
    set_trace_refpos(uistat.current_trace, uistat.value / 1000.0);
    break;
  case KM_EDELAY:
    set_electrical_delay(uistat.value);
    break;
  case KM_VELOCITY_FACTOR:
    velocity_factor = uistat.value;
    break;
  }
}

#endif

static void draw_numeric_area(void)
{
  char buf[10];
  snprintf(buf, sizeof buf, "%9d", uistat.value);
  draw_numeric_input(buf);
}


static void ui_mode_menu(void)
{
  if (ui_mode == UI_MENU) 
    return;

  ui_mode = UI_MENU;
  /* narrowen plotting area */
  #if !defined(ANTENNA_ANALYZER)
//  area_width = AREA_WIDTH_NORMAL - (64-8);
  #else
   area_width = AREA_WIDTH_NORMAL - 72;
  #endif
//  area_height = HEIGHT;
  ensure_selection();
  draw_menu();
}

static void ui_mode_numeric(int _keypad_mode)
{
  if (ui_mode == UI_NUMERIC) 
    return;

  leave_ui_mode();
  
  // keypads array
  keypad_mode = _keypad_mode;
  ui_mode = UI_NUMERIC;
//  area_width = AREA_WIDTH_NORMAL;
//  area_height = 240-32;//HEIGHT - 32;

  draw_numeric_area_frame();
  fetch_numeric_target();
  draw_numeric_area();
}

static void ui_mode_keypad(int _keypad_mode)
{
  if (ui_mode == UI_KEYPAD) 
    return;

  // keypads array
  keypad_mode = _keypad_mode;
  keypads = keypads_mode_tbl[_keypad_mode];
  int i;
  for (i = 0; keypads[i+1].c >= 0; i++)
    ;
  keypads_last_index = i;

  ui_mode = UI_KEYPAD;
//  area_width = AREA_WIDTH_NORMAL - (64-8);
//  area_height = HEIGHT - 32;
  draw_menu();
  draw_keypad();
  draw_numeric_area_frame();
  draw_numeric_input("");
}

static void ui_mode_normal(void)
{
  if (ui_mode == UI_NORMAL) 
    return;

//  area_width = AREA_WIDTH_NORMAL;
//  area_height = HEIGHT;
  leave_ui_mode();
  ui_mode = UI_NORMAL;
}

/*
static void ui_process_normal(void)
{
  int status = btn_check();
  if (status != 0) {
    if (status & EVT_BUTTON_SINGLE_CLICK) {
      ui_mode_menu();
    } else {
      do {
        if (active_marker >= 0 && markers[active_marker].enabled) {
          if ((status & EVT_DOWN) && markers[active_marker].index > 0) {
            markers[active_marker].index--;
            markers[active_marker].frequency = frequencies[markers[active_marker].index];
            redraw_marker(active_marker, FALSE);
          }
          if ((status & EVT_UP) && markers[active_marker].index < (POINT_COUNT-1)) {
            markers[active_marker].index++;
            markers[active_marker].frequency = frequencies[markers[active_marker].index];
            redraw_marker(active_marker, FALSE);
          }
        }
        status = btn_wait_release();
      } while (status != 0);
      if (active_marker >= 0)
//        redraw_marker(active_marker, TRUE);
    }
  }
}
/*
static void ui_process_menu(void)
{
  int status = btn_check();
  if (status != 0) {
    if (status & EVT_BUTTON_SINGLE_CLICK) {
      menu_invoke(selection);
    } else {
      do {
        if (status & EVT_UP){
            if (menu_stack[menu_current_level][selection+1].type == MT_NONE) {
              ui_mode_normal();
              return;
            }
              selection++;
          draw_menu();
        }
        if (status & EVT_DOWN){
            if (selection == 0){
              ui_mode_normal();
              return;}
          selection--;
          draw_menu();
        }
        status = btn_wait_release();
      } while (status != 0);
    }
  }
}
*/

double my_atof(const char *p)
{
  int neg = FALSE;
  if (*p == '-')
    neg = TRUE;
  if (*p == '-' || *p == '+')
    p++;
    double x = atoi(p);
  while (isdigit((int)*p))
    p++;
  if (*p == '.') {
        double d = 1.0f;
    p++;
    while (isdigit((int)*p)) {
      d /= 10;
      x += d * (*p - '0');
      p++;
    }
  }
  if (*p == 'e' || *p == 'E') {
    p++;
    int exp = atoi(p);
    while (exp > 0) {
      x *= 10;
      exp--;
    }
    while (exp < 0) {
      x /= 10;
      exp++;
    }
  }
  if (neg)
    x = -x;
  return x;
}

static int keypad_click(int key) 
{
  int c = keypads[key].c;
  if ((c >= KP_X1 && c <= KP_G) || c == KP_N || c == KP_P) {
    int32_t scale = 1;
    if (c >= KP_X1 && c <= KP_G) {
      int n = c - KP_X1;
      while (n-- > 0)
        scale *= 1000;
    } else if (c == KP_N) {
      scale *= 1000;
    }
    /* numeric input done */
    double value = my_atof(kp_buf) * (double)scale;
    switch (keypad_mode) {
    case KM_START:
      set_sweep_frequency(ST_START, (int32_t)value);
      break;
    case KM_STOP:
      set_sweep_frequency(ST_STOP, (int32_t)value);
      break;
    case KM_CENTER:
      set_sweep_frequency(ST_CENTER, (int32_t)value);
      break;
    case KM_SPAN:
      set_sweep_frequency(ST_SPAN, (int32_t)value);
      break;
    case KM_CW:
      set_sweep_frequency(ST_CW, (int32_t)value);
      break;
    case KM_SCALE:
//      set_trace_scale(uistat.current_trace, value);
      break;
    case KM_REFPOS:
//      set_trace_refpos(uistat.current_trace, value);
      break;
    case KM_EDELAY:
//      set_electrical_delay(value); // pico seconds
      break;
    case KM_VELOCITY_FACTOR:
//      velocity_factor = (uint8_t)value;
      break;
    case KM_SCALEDELAY:
//      set_trace_scale(uistat.current_trace, value * 1e-12); // pico second
      break;
    }

    return KP_DONE;
  } else if (c <= 9 && kp_index < NUMINPUT_LEN)
    kp_buf[kp_index++] = '0' + c;
  else if (c == KP_PERIOD && kp_index < NUMINPUT_LEN) {
    // check period in former input
    int j;
    for (j = 0; j < kp_index && kp_buf[j] != '.'; j++)
      ;
    // append period if there are no period
    if (kp_index == j)
      kp_buf[kp_index++] = '.';
  } else if (c == KP_MINUS) {
    if (kp_index == 0)
      kp_buf[kp_index++] = '-';    
  } else if (c == KP_BS) {
    if (kp_index == 0) {
      return KP_CANCEL;
    }
    --kp_index;
  }
  kp_buf[kp_index] = '\0';
  draw_numeric_input(kp_buf);
  return KP_CONTINUE;
}

static int keypad_apply_touch(void)
{
  int touch_x, touch_y;
  int i = 0;

  touch_position(&touch_x, &touch_y);

  while (keypads[i].x) {
    if (keypads[i].x-2 < touch_x && touch_x < keypads[i].x+44+2
        && keypads[i].y-2 < touch_y && touch_y < keypads[i].y+44+2) {
      // draw focus
      selection = i;
      draw_keypad();
      touch_wait_release();
      // erase focus
      selection = -1;
      draw_keypad();
      return i;
    }
    i++;
  }
  if (touch_y > 48 * 4) {
    // exit keypad mode
    return -2;
  }
  return -1;
}

static void numeric_apply_touch(void)
{
  int touch_x, touch_y;
  touch_position(&touch_x, &touch_y);

  if (touch_x < 64) {
    ui_mode_normal();
    return;
  }
  if (touch_x > 64+9*20+8+8) {
    ui_mode_keypad(keypad_mode);
    ui_process_keypad();
    return;
  }

  if (touch_y > 240-40) {
    int n = 9 - (touch_x - 64) / 20;
    uistat.digit = n;
    uistat.digit_mode = TRUE;
  } else {
    int step, n;
    if (touch_y < 100) {
      step = 1;
    } else {
      step = -1;
    }

    for (n = uistat.digit; n > 0; n--)
      step *= 10;
    uistat.value += step;
  }
  draw_numeric_area();
  
  touch_wait_release();
  uistat.digit_mode = FALSE;
  draw_numeric_area();
  
  return;
}

#if 0
static void ui_process_numeric(void)
{
  int status = btn_check();

  if (status != 0) {
    if (status == EVT_BUTTON_SINGLE_CLICK) {
      status = btn_wait_release();
      if (uistat.digit_mode) {
        if (status & (EVT_BUTTON_SINGLE_CLICK | EVT_BUTTON_DOWN_LONG)) {
          uistat.digit_mode = FALSE;
          draw_numeric_area();
        }
      } else {
        if (status & EVT_BUTTON_DOWN_LONG) {
          uistat.digit_mode = TRUE;
          draw_numeric_area();
        } else if (status & EVT_BUTTON_SINGLE_CLICK) {
          set_numeric_value();
          ui_mode_normal();
        }
      }
    } else {
      do {
        if (uistat.digit_mode) {
          if (status & EVT_DOWN) {
            if (uistat.digit < 8) {
              uistat.digit++;
              draw_numeric_area();
            } else {
              goto exit;
            }
          }
          if (status & EVT_UP) {
            if (uistat.digit > 0) {
              uistat.digit--;
              draw_numeric_area();
            } else {
              goto exit;
            }
          }
        } else {
          int32_t step = 1;
          int n;
          for (n = uistat.digit; n > 0; n--)
            step *= 10;
          if (status & EVT_DOWN) {
            uistat.value += step;
            draw_numeric_area();
          }
          if (status & EVT_UP) {
            uistat.value -= step;
            draw_numeric_area();
          }
        }
        status = btn_wait_release();
      } while (status != 0);
    }
  }

  return;

 exit:
  // cancel operation
  ui_mode_normal();
}
#endif


static void ui_process_keypad(void)
{
  int status;

  kp_index = 0;
  while (TRUE) {

    status = touch_check();
    if (status == EVT_TOUCH_PRESSED) {
      int key = keypad_apply_touch();
      if (key >= 0 && keypad_click(key))
        /* exit loop on done or cancel */
        break;
      else if (key == -2) {
        //xxx;
 //       return;
      }
    }
  }

  redraw_frame();
  request_to_redraw_grid();
  ui_mode_normal();
  //redraw_all();
  touch_start_watchdog();
}
/*
static void ui_process_lever(void)
{
  switch (ui_mode) {
  case UI_NORMAL:
    ui_process_normal();
    break;    
  case UI_MENU:
    ui_process_menu();
    break;    
//  case UI_NUMERIC:
//    ui_process_numeric();
//    break;
  case UI_KEYPAD:
    ui_process_keypad();
    break;    
  }
}
*/

static void drag_marker(int t, int m)
{
  int status;
  /* wait touch release */
  do {
    int touch_x, touch_y;
    int index;
    touch_position(&touch_x, &touch_y);
    touch_x -= OFFSETX;
    touch_y -= OFFSETY;
//    index = search_nearest_index(touch_x, touch_y, t);
    if (index >= 0) {
      markers[m].index = index;
      markers[m].frequency = frequencies(index);
//      redraw_marker(m, TRUE);
    }

    status = touch_check();
  } while(status != EVT_TOUCH_RELEASED);
}

static int sq_distance(int x0, int y0)
{
  return x0*x0 + y0*y0;
}

static int touch_pickup_marker(void)
{
  int touch_x, touch_y;
  int m, t;
  touch_position(&touch_x, &touch_y);
  touch_x -= OFFSETX;
  touch_y -= OFFSETY;

  for (m = 0; m < MARKER_COUNT; m++) {
    if (!markers[m].enabled)
      continue;

    for (t = 0; t < TRACE_COUNT; t++) {
      int x, y;
      if (!trace[t].enabled)
        continue;

//      marker_position(m, t, &x, &y);

      if (sq_distance(x - touch_x, y - touch_y) < 400) {
        if (active_marker != m) {
          previous_marker = active_marker;
          active_marker = m;
//          redraw_marker(active_marker, TRUE);
        }
        // select trace
        uistat.current_trace = t;
        
        // drag marker until release
        drag_marker(t, m);
        return TRUE;
      }
    }
  }

  return FALSE;
}


void ui_process_touch(void)
{
  awd_count++;

  int status = touch_check();
  if (status == EVT_TOUCH_PRESSED || status == EVT_TOUCH_DOWN) {
    switch (ui_mode) {
    case UI_NORMAL:

      if (touch_pickup_marker()) {
        break;
      }
      
      touch_wait_release();

      // switch menu mode
      selection = -1;
      ui_mode_menu();
      break;

    case UI_MENU:
      menu_apply_touch();
      break;

    case UI_NUMERIC:
      numeric_apply_touch();
      break;
    }
  }
  touch_start_watchdog();
}

void ui_process(void)
{
  switch (operation_requested) {
  case OP_TOUCH:
    ui_process_touch();
    break;
  }
  operation_requested = OP_NONE;
}
