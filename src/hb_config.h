#ifndef HB_CONFIG_H
#define HB_CONFIG_H

#include <glib.h>
#include <json-glib/json-glib.h>

// ---------------- Config parsing (JSON) ----------------

typedef struct
{
    gchar* title;
    gchar* address;
    gint width;
    gint height;
    gboolean locked;
    gdouble opacity;
    gboolean transparent; // if true, make window and web content backgrounds transparent
} HbWindowCfg;

void hb_window_cfg_init_defaults(HbWindowCfg* cfg);
void hb_window_cfg_clear(HbWindowCfg* cfg);

// Load configuration(s) from a JSON file. Returns GPtrArray* of HbWindowCfg* on success; NULL on failure.
GPtrArray* hb_load_configs_from_json(const gchar* path);

// Write the default configuration JSON to the given path. Returns TRUE on success.
gboolean hb_write_default_config(const gchar* path);

// Ensure a default config exists at path; creates one if missing.
void hb_ensure_default_config_exists(const gchar* path);

#endif // HB_CONFIG_H
