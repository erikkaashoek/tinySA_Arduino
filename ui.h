#include <XPT2046_Touchscreen.h>
extern XPT2046_Touchscreen ts;

void  ili9341_drawstring_size(const char *t, int x,int y, int tc, int bc, int size);

enum {
  ST_START, ST_STOP, ST_CENTER, ST_SPAN, ST_CW
};

void set_sweep_frequency(int type, int32_t frequency);
uint32_t get_sweep_frequency(int type);
void clearDisplay(void);
void ui_process_touch(void);
void SetPowerGrid(int);
void SetRefLevel(int);
void set_refer_output(int);
void SetAttenuation(int);
void SetPowerLevel(int);
void SetGenerate(int);
void SetRX(int);
void SetRBW(int);
void SetSpur(int);
void SetAverage(int);
void  SetStorage(void);
void  SetClearStorage(void);
void  SetSubtractStorage(void);
void AllDirty(void);
void MenuDirty(void);
void redrawHisto(void);

enum {
  AV_OFF, AV_MIN, AV_MAX, AV_2, AV_4, AV_8
};

enum {
  UI_NORMAL, UI_MENU, UI_NUMERIC, UI_KEYPAD
};

extern uint8_t ui_mode;


#define TRACE_COUNT 1


typedef struct {
    int32_t magic;
#ifdef __DAC__
    uint16_t dac_value;
#endif
    uint16_t grid_color;
    uint16_t menu_normal_color;
    uint16_t menu_active_color;
    uint16_t trace_color[TRACE_COUNT];
    int16_t touch_cal[4];
    int8_t default_loadcal;
    uint32_t harmonic_freq_threshold;
    int16_t vbat_offset;
    int32_t checksum;
} config_t;


extern config_t config;
