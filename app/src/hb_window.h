#ifndef HB_WINDOW_H
#define HB_WINDOW_H

#include <gtk/gtk.h>
#include "hb_config.h"

// Create and present a window according to the given configuration.
void hb_create_window(GtkApplication* app, const HbWindowCfg* cfg);

#endif // HB_WINDOW_H
