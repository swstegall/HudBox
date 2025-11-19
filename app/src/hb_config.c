#include "hb_config.h"
#include <glib.h>
#include <json-glib/json-glib.h>

void hb_window_cfg_init_defaults(HbWindowCfg* cfg)
{
    cfg->title = g_strdup("HudBox");
    cfg->address = g_strdup("https://swstegall.github.io/HudBox/");
    cfg->width = 800;
    cfg->height = 600;
    cfg->locked = FALSE;
    cfg->opacity = 0.9;
    cfg->transparent = FALSE;
}

void hb_window_cfg_clear(HbWindowCfg* cfg)
{
    if (!cfg) return;
    g_clear_pointer(&cfg->title, g_free);
    g_clear_pointer(&cfg->address, g_free);
}

static void hb_apply_object_to_cfg(JsonObject* obj, HbWindowCfg* cfg)
{
    if (!obj || !cfg) return;

    // title
    if (json_object_has_member(obj, "title"))
    {
        const gchar* s = json_object_get_string_member(obj, "title");
        if (s)
        {
            g_free(cfg->title);
            cfg->title = g_strdup(s);
        }
    }
    // address (aka uri)
    if (json_object_has_member(obj, "address"))
    {
        const gchar* s = json_object_get_string_member(obj, "address");
        if (s)
        {
            g_free(cfg->address);
            cfg->address = g_strdup(s);
        }
    }
    else if (json_object_has_member(obj, "uri"))
    {
        const gchar* s = json_object_get_string_member(obj, "uri");
        if (s)
        {
            g_free(cfg->address);
            cfg->address = g_strdup(s);
        }
    }
    // width/height
    if (json_object_has_member(obj, "width"))
    {
        cfg->width = (gint)json_object_get_int_member(obj, "width");
    }
    if (json_object_has_member(obj, "height"))
    {
        cfg->height = (gint)json_object_get_int_member(obj, "height");
    }
    // locked
    if (json_object_has_member(obj, "locked"))
    {
        cfg->locked = json_object_get_boolean_member(obj, "locked");
    }
    // opacity
    if (json_object_has_member(obj, "opacity"))
    {
        cfg->opacity = json_object_get_double_member(obj, "opacity");
    }
    // transparent
    if (json_object_has_member(obj, "transparent"))
    {
        cfg->transparent = json_object_get_boolean_member(obj, "transparent");
    }
}

GPtrArray* hb_load_configs_from_json(const gchar* path)
{
    gchar* contents = NULL;
    gsize len = 0;
    GError* error = NULL;
    if (!g_file_get_contents(path, &contents, &len, &error))
    {
        if (error) g_error_free(error);
        return NULL;
    }

    JsonParser* parser = json_parser_new();
    if (!json_parser_load_from_data(parser, contents, len, &error))
    {
        if (error) g_error_free(error);
        g_object_unref(parser);
        g_free(contents);
        return NULL;
    }

    JsonNode* root = json_parser_get_root(parser);
    if (!root)
    {
        g_object_unref(parser);
        g_free(contents);
        return NULL;
    }

    GPtrArray* arr = g_ptr_array_new_with_free_func(g_free);

    if (JSON_NODE_HOLDS_ARRAY(root))
    {
        JsonArray* ja = json_node_get_array(root);
        guint n = json_array_get_length(ja);
        for (guint i = 0; i < n; i++)
        {
            JsonObject* obj = json_array_get_object_element(ja, i);
            if (!obj) continue;
            HbWindowCfg* cfg = g_new0(HbWindowCfg, 1);
            hb_window_cfg_init_defaults(cfg);
            hb_apply_object_to_cfg(obj, cfg);
            // clamp opacity
            if (cfg->opacity < 0.0) cfg->opacity = 0.0;
            if (cfg->opacity > 1.0) cfg->opacity = 1.0;
            g_ptr_array_add(arr, cfg);
        }
    }
    else if (JSON_NODE_HOLDS_OBJECT(root))
    {
        JsonObject* obj = json_node_get_object(root);
        HbWindowCfg* cfg = g_new0(HbWindowCfg, 1);
        hb_window_cfg_init_defaults(cfg);
        hb_apply_object_to_cfg(obj, cfg);
        if (cfg->opacity < 0.0) cfg->opacity = 0.0;
        if (cfg->opacity > 1.0) cfg->opacity = 1.0;
        g_ptr_array_add(arr, cfg);
    }

    g_object_unref(parser);
    g_free(contents);

    if (arr->len == 0)
    {
        g_ptr_array_free(arr, TRUE);
        return NULL;
    }
    return arr;
}

gboolean hb_write_default_config(const gchar* path)
{
    if (!path) return FALSE;
    const char* json =
        "[\n"
        "\t{\n"
        "\t  \"title\": \"HudBox\",\n"
        "\t  \"address\": \"https://swstegall.github.io/HudBox/\",\n"
        "\t  \"width\": 800,\n"
        "\t  \"height\": 600,\n"
        "\t  \"locked\": false,\n"
        "\t  \"opacity\": 0.9,\n"
        "\t  \"transparent\": false\n"
        "\t}\n"
        "]\n";
    GError* error = NULL;
    gboolean ok = g_file_set_contents(path, json, -1, &error);
    if (!ok)
    {
        if (error)
        {
            g_warning("Failed to write default config to %s: %s", path, error->message);
            g_error_free(error);
        }
    }
    return ok;
}

void hb_ensure_default_config_exists(const gchar* path)
{
    if (!path) return;
    if (!g_file_test(path, G_FILE_TEST_EXISTS))
    {
        hb_write_default_config(path);
    }
}
