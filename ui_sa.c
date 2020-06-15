#ifdef __SA__


void set_sweep_frequency(int type, int32_t frequency);
uint32_t get_sweep_frequency(int type);
void clearDisplay(void);
//void ui_process_touch(void);
void SetPowerGrid(int);
void SetRefLevel(int);
void set_refer_output(int);
void SetAttenuation(int);
void SetPowerLevel(int);
void SetGenerate(int);
void SetRBW(int);
void SetSpur(int);
void SetAverage(int);
void  SetStorage(void);
void  SetClearStorage(void);
void  SetSubtractStorage(void);
void AllDirty(void);
void MenuDirty(void);
void redrawHisto(void);
void self_test(void);


enum {
  KM_START, KM_STOP, KM_CENTER, KM_SPAN, KM_CW, KM_REFPOS, KM_SCALE, KM_ATTENUATION, KM_ACTUALPOWER
};


#define KP_X(x) (48*(x) + 2 + (320-64-192))
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

static const keypads_t keypads_level[] = {
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
  { KP_X(3), KP_Y(2), KP_MINUS },
  { KP_X(3), KP_Y(3), KP_X1 },
  { KP_X(2), KP_Y(3), KP_BS },
  { 0, 0, -1 }
};


static const keypads_t * const keypads_mode_tbl[] = {
  keypads_freq, // start
  keypads_freq, // stop
  keypads_freq, // center
  keypads_freq, // span
  keypads_freq, // cw freq
  keypads_level, // refpos
  keypads_scale, // scale
  keypads_scale, // attenuation
  keypads_level, // actual power
};

#ifdef __VNA__
static const char * const keypad_mode_label[] = {
  "START", "STOP", "CENTER", "SPAN", "CW FREQ", "SCALE", "REFPOS", "EDELAY", "VELOCITY%", "DELAY"
};
#endif
#ifdef __SA__
static const char * const keypad_mode_label[] = {
  "START", "STOP", "CENTER", "SPAN", "CW FREQ", "REFPOS", "SCALE", "ATTENUATION", "ACTUALPOWER"
};
#endif


// ===[MENU CALLBACKS]=========================================================


int generator_enabled = false;

static void menu_mode_cb(int item)
{
  switch (item) {
  case 4: // Change reference output
    break;
  default:
    SetMode(item);
    menu_move_back();
    ui_mode_normal();
    draw_cal_status();
    break;
  }

}

extern int dirty;
void menu_autosettings_cb(int item)
{
  SetMode(M_LOW);
//  set_sweep_frequency(ST_START, (int32_t) 0);
//  set_sweep_frequency(ST_STOP, (int32_t) 300000000);

  int value = 10; // 10dB/
  set_trace_scale(0, value);
  set_trace_scale(1, value);
  set_trace_scale(2, value);

  value = -10; // Top at -10dB
  set_trace_refpos(0, - value / get_trace_scale(0) + YGRIDS);
  set_trace_refpos(1, - value / get_trace_scale(0) + YGRIDS);
  set_trace_refpos(2, - value / get_trace_scale(0) + YGRIDS);

  set_refer_output(1);
  SetAttenuation(0);
  SetPowerLevel(100); // Reset
  SetRBW(0);
  dirty = true;
  menu_move_back();
  ui_mode_normal();
  draw_cal_status();
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
      menu_move_back();
      ui_mode_normal();
      self_test();
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

int menu_refer_value[]={-1,0,1,2,3,4,5,6};
static void menu_refer_cb(int item)
{
//Serial.println(item);
  set_refer_output(menu_refer_value[item]); 
  menu_move_back();
  ui_mode_normal();
  draw_cal_status();
}

static void menu_refer_cb2(int item)
{
//Serial.println(item);
  set_refer_output(menu_refer_value[item+5]); 
  menu_move_back();
  ui_mode_normal();
  draw_cal_status();
}

static void menu_spur_cb(int item)
{
  SetSpur(item); 
  menu_move_back();
  ui_mode_normal();
  draw_cal_status();
}

static void menu_storage_cb(int item)
{
  switch(item) {
    case 0:
      SetStorage();
      break;
    case 1:
      SetClearStorage();
      break;
    case 2:
      SetSubtractStorage();
      break;
  }
  menu_move_back();
  ui_mode_normal();
  draw_cal_status();
}

static void menu_average_cb(int item)
{
  SetAverage(item); 
  menu_move_back();
  ui_mode_normal();
  draw_cal_status();
}


int rbwsel[]={0,3,10,30,100,300};

static void menu_rbw_cb(int item)
{
  SetRBW(rbwsel[item]); 
  menu_move_back();
  ui_mode_normal();
  draw_cal_status();
}

int menu_dBper_value[]={1,2,5,10,20};

static void menu_dBper_cb(int item)
{
  set_trace_scale(0, menu_dBper_value[item]);
  set_trace_scale(1, menu_dBper_value[item]);
  set_trace_scale(2, menu_dBper_value[item]);
  menu_move_back();
  ui_mode_normal();
  draw_cal_status();
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
#if 0
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
#endif

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
  int km = KM_REFPOS + item;
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

static void menu_actualpower_cb(int item)
{
  int status;
  int km = KM_ACTUALPOWER;
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
    toggle_sweep();
    menu_move_back();
    ui_mode_normal();
    draw_menu();
    break;
  }
}

static void menu_marker_sel_cb(int);
static void menu_marker_op_cb(int);
static void menu_marker_sel_cb(int);

// ===[MENU DEFINITION]=========================================================


static const menuitem_t menu_average[] = {
  MENUITEM_FUNC("OFF",   menu_average_cb),
  MENUITEM_FUNC("MIN",   menu_average_cb),
  MENUITEM_FUNC("MAX",   menu_average_cb),
  MENUITEM_FUNC(" 2 ",   menu_average_cb),
  MENUITEM_FUNC(" 4 ",   menu_average_cb),
  MENUITEM_FUNC(" 8 ",   menu_average_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_storage[] = {
  MENUITEM_FUNC("STORE",    menu_storage_cb),
  MENUITEM_FUNC("CLEAR",    menu_storage_cb),
  MENUITEM_FUNC("SUBTRACT", menu_storage_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_spur[] = {
  MENUITEM_FUNC("OFF",   menu_spur_cb),
  MENUITEM_FUNC("ON",    menu_spur_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_rbw[] = {
  MENUITEM_FUNC("  AUTO",   menu_rbw_cb),
  MENUITEM_FUNC("  3kHz",   menu_rbw_cb),
  MENUITEM_FUNC(" 10kHz",   menu_rbw_cb),
  MENUITEM_FUNC(" 30kHz",   menu_rbw_cb),
  MENUITEM_FUNC("100kHz",   menu_rbw_cb),
  MENUITEM_FUNC("300kHz",   menu_rbw_cb),
  MENUITEM_BACK,
  MENUITEM_END
};


static const menuitem_t menu_dBper[] = {
  MENUITEM_FUNC("  1dB/",   menu_dBper_cb),
  MENUITEM_FUNC("  2dB/",   menu_dBper_cb),
  MENUITEM_FUNC("  5dB/",   menu_dBper_cb),
  MENUITEM_FUNC(" 10dB/",   menu_dBper_cb),
  MENUITEM_FUNC(" 20dB/",   menu_dBper_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

#if 0
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
#endif

static const menuitem_t menu_refer2[] = {
  MENUITEM_FUNC("3MHz" ,   menu_refer_cb2),
  MENUITEM_FUNC("2MHz" ,   menu_refer_cb2),
  MENUITEM_FUNC("1MHz" ,   menu_refer_cb2),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_refer[] = {
  MENUITEM_FUNC("OFF"  ,   menu_refer_cb),
  MENUITEM_FUNC("30MHz",   menu_refer_cb),
  MENUITEM_FUNC("15MHz",   menu_refer_cb),
  MENUITEM_FUNC("10MHz",   menu_refer_cb),
  MENUITEM_FUNC("4MHz" ,   menu_refer_cb),
  MENUITEM_MENU(S_RARROW" MORE", menu_refer2),  
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_scale[] = {
  MENUITEM_FUNC("\2REF\0LEVEL",  menu_scale_cb),
  MENUITEM_MENU("SCALE/DIV",     menu_dBper),
  MENUITEM_FUNC("ATTEN",         menu_scale_cb),
  MENUITEM_MENU("AVERAGE",       menu_average),
  MENUITEM_MENU("\2SPUR\0REDUCTION",menu_spur),
  MENUITEM_MENU("RBW",           menu_rbw),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_stimulus[] = {
  MENUITEM_FUNC("START",            menu_stimulus_cb),
  MENUITEM_FUNC("STOP",             menu_stimulus_cb),
  MENUITEM_FUNC("CENTER",           menu_stimulus_cb),
  MENUITEM_FUNC("SPAN",             menu_stimulus_cb),
  MENUITEM_FUNC("CW FREQ",          menu_stimulus_cb),
//  MENUITEM_MENU("RBW",              menu_rbw),
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

static const menuitem_t menu_dfu[] = {
  MENUITEM_FUNC("ENTER DFU", menu_dfu_cb),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_mode[] = {
  MENUITEM_FUNC("\2LOW\0INPUT", menu_mode_cb),
  MENUITEM_FUNC("\2HIGH\0INPUT",menu_mode_cb),
  MENUITEM_FUNC("\2LOW\0OUTPUT", menu_mode_cb),
  MENUITEM_FUNC("\2HIGH\0OUTPUT",menu_mode_cb),
  MENUITEM_MENU("\2REFERENCE\0OUTPUT",menu_refer),
  MENUITEM_BACK,
  MENUITEM_END
};


static const menuitem_t menu_config[] = {
  MENUITEM_FUNC("TOUCH CAL",    menu_config_cb),
  MENUITEM_FUNC("TOUCH TEST",   menu_config_cb),
  MENUITEM_FUNC("SELF TEST",    menu_config_cb),
  MENUITEM_FUNC("VERSION",      menu_config_cb),
  MENUITEM_FUNC("\2ACTUAL\0POWER",menu_actualpower_cb),
//  MENUITEM_MENU("RBW", menu_rbw),
  MENUITEM_MENU(S_RARROW"DFU",  menu_dfu),
  MENUITEM_BACK,
  MENUITEM_END
};

static const menuitem_t menu_top[] = {
  MENUITEM_FUNC("\2AUTO\0SETTINGS",  menu_autosettings_cb),
  MENUITEM_MENU("SCAN",       menu_stimulus),
  MENUITEM_MENU("MARKER",     menu_marker),
  MENUITEM_MENU("DISPLAY",    menu_scale),
  MENUITEM_MENU("STORAGE",    menu_storage),
  MENUITEM_MENU("MODE",       menu_mode),
  MENUITEM_MENU("CONFIG",     menu_config),
  MENUITEM_END,
 // MENUITEM_CLOSE,
};

// ===[MENU DEFINITION END]======================================================

#undef BOARD_NAME
#define BOARD_NAME  "tinySA"


void
show_logo(void)
{
  int x = 15, y = 30;
  ili9341_fill(0, 0, 320, 240, 0);
  ili9341_drawstring_size(BOARD_NAME, x+60, y, RGBHEX(0x0000FF), 0x0000, 4);
  y += 25;

  ili9341_drawstring_size("TINYSA.ORG", x+100, y += 10, 0xffff, 0x0000, 2);
  ili9341_drawstring_5x7("https://github.com/erikkaashoek/tinySA", x, y += 20, 0xffff, 0x0000);
  ili9341_drawstring_5x7("Based on edy555 design", x, y += 10, 0xffff, 0x0000);
  ili9341_drawstring_5x7("2016-2019 Copyright @edy555", x, y += 10, 0xffff, 0x0000);
  ili9341_drawstring_5x7("Licensed under GPL. See: https://github.com/ttrftech/NanoVNA", x, y += 10, 0xffff, 0x0000);
  ili9341_drawstring_5x7("Version: " VERSION, x, y += 10, 0xffff, 0x0000);
  ili9341_drawstring_5x7("Build Time: " __DATE__ " - " __TIME__, x, y += 10, 0xffff, 0x0000);
//  y += 5;
//  ili9341_drawstring_5x7("Kernel: " CH_KERNEL_VERSION, x, y += 10, 0xffff, 0x0000);
//  ili9341_drawstring_5x7("Architecture: " PORT_ARCHITECTURE_NAME " Core Variant: " PORT_CORE_VARIANT_NAME, x, y += 10, 0xffff, 0x0000);
//  ili9341_drawstring_5x7("Port Info: " PORT_INFO, x, y += 10, 0xffff, 0x0000);
//  ili9341_drawstring_5x7("Platform: " PLATFORM_NAME, x, y += 10, 0xffff, 0x0000);
}


#endif



static void menu_item_modify_attribute(
    const menuitem_t *menu, int item, uint16_t *fg, uint16_t *bg)
{
#ifdef __VNA__
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
      domain_mode = (domain_mode & ~DOMAIN_MODE) | DOMAIN_FREQ;
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
  } else if (menu == menu_output) {
      if ((item == 1 && generator_enabled){
        *bg = 0x0000;
        *fg = 0xffff;
      }
  } else if (menu == menu_transform_window) {
      if ((item == 0 && (domain_mode & TD_WINDOW) == TD_WINDOW_MINIMUM)
       || (item == 1 && (domain_mode & TD_WINDOW) == TD_WINDOW_NORMAL)
       || (item == 2 && (domain_mode & TD_WINDOW) == TD_WINDOW_MAXIMUM)
       ) {
        *bg = 0x0000;
        *fg = 0xffff;
      }
  }
#endif
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
    uistat.value = get_trace_scale(uistat.current_trace) * 1000;
    break;
  case KM_REFPOS:
    uistat.value = get_trace_refpos(uistat.current_trace) * 1000;
    break;
#ifdef __VNA__
  case KM_EDELAY:
    uistat.value = get_electrical_delay();
    break;
  case KM_VELOCITY_FACTOR:
    uistat.value = velocity_factor;
    break;
  case KM_SCALEDELAY:
    uistat.value = get_trace_scale(uistat.current_trace) * 1e12;
    break;
#endif
#ifdef __SA__
  case KM_ATTENUATION:
    uistat.value = settingAttenuate;
     break;
  case KM_ACTUALPOWER:
    uistat.value = settingPowerCal;
    break;
#endif

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
      set_trace_scale(0, value);
      set_trace_scale(1, value);
      set_trace_scale(2, value);
      break;
    case KM_REFPOS:
      set_trace_refpos(0, YGRIDS - value / get_trace_scale(0));
      set_trace_refpos(1, YGRIDS - value / get_trace_scale(0));
      set_trace_refpos(2, YGRIDS - value / get_trace_scale(0));
      break;
    case KM_ATTENUATION:
       SetAttenuation(value);
       break;
    case KM_ACTUALPOWER:
      SetPowerLevel(value);
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
