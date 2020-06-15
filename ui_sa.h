#include "tinySA.h"


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
