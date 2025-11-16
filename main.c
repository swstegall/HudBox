#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <glib.h>
#include <json-glib/json-glib.h>
#include <stdlib.h>

// Apply a global CSS to make GTK toplevel windows fully transparent
static void hb_install_transparent_css(void)
{
    // Only windows that opt-in via CSS class "hb-transparent" become transparent
    const char* css =
        ".hb-transparent { background-color: transparent; }\n"
        "window.hb-transparent { background-color: transparent; }\n"
        "GtkWindow.hb-transparent { background-color: transparent; }\n";
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, css);
    GdkDisplay* display = gdk_display_get_default();
    if (display)
    {
        gtk_style_context_add_provider_for_display(
            display,
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    g_object_unref(provider);
}

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

static void hb_window_cfg_init_defaults(HbWindowCfg* cfg)
{
    cfg->title = g_strdup("Borderless Draggable WebView");
    cfg->address = g_strdup("https://www.google.com");
    cfg->width = 800;
    cfg->height = 325;
    cfg->locked = FALSE;
    cfg->opacity = 1.0;
    cfg->transparent = FALSE;
}

static void hb_window_cfg_clear(HbWindowCfg* cfg)
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

static GPtrArray* hb_load_configs_from_json(const gchar* path)
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

// ---------------- Drag handling ----------------

// Called when user starts dragging
static void on_drag_begin(GtkGestureClick* gesture,
                          gint n_press,
                          gdouble x,
                          gdouble y,
                          gpointer user_data)
{
    GtkWindow* window = GTK_WINDOW(user_data);

    // GTK 4 replacement: get seat/device and start a system move
    GdkSurface* surface = gtk_native_get_surface(gtk_widget_get_native(GTK_WIDGET(window)));
    GdkDevice* device = gtk_gesture_get_device(GTK_GESTURE(gesture));
    if (surface && device)
    {
        gdk_toplevel_begin_move(GDK_TOPLEVEL(surface), device, n_press, x, y, GDK_CURRENT_TIME);
    }
}

static void hb_create_window(GtkApplication* app, const HbWindowCfg* cfg)
{
    GtkWidget* window = gtk_application_window_new(app);

    gtk_window_set_title(GTK_WINDOW(window), cfg->title);
    gtk_window_set_default_size(GTK_WINDOW(window), cfg->width, cfg->height);

    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    // Clamp and apply opacity
    gdouble opacity = cfg->opacity;
    if (opacity < 0.0) opacity = 0.0;
    if (opacity > 1.0) opacity = 1.0;
    gtk_widget_set_opacity(window, opacity);

    // WebView (conditionally make page/window transparent based on config)
    GtkWidget* web_view = webkit_web_view_new();

    if (cfg->transparent)
    {
        // Mark the GTK window as transparent via CSS class
        gtk_widget_add_css_class(window, "hb-transparent");

        // Force page background to be transparent
        const char* css_page_transparent =
            "html, body {\n"
            "  background: transparent !important;\n"
            "  background-color: transparent !important;\n"
            "}\n";
        WebKitUserStyleSheet* sheet = webkit_user_style_sheet_new(
            css_page_transparent,
            WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
            WEBKIT_USER_STYLE_LEVEL_USER,
            NULL, NULL);
        WebKitUserContentManager* ucm = webkit_web_view_get_user_content_manager(WEBKIT_WEB_VIEW(web_view));
        webkit_user_content_manager_add_style_sheet(ucm, sheet);
        g_object_unref(sheet);

        // Transparent WebView surface to let window show through
        GdkRGBA transparent = (GdkRGBA){0, 0, 0, 0};
        webkit_web_view_set_background_color(WEBKIT_WEB_VIEW(web_view), &transparent);
    }

    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), cfg->address);

    if (!cfg->locked)
    {
        GtkGesture* drag = gtk_gesture_click_new();
        gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(drag), GTK_PHASE_BUBBLE);
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(drag), GDK_BUTTON_PRIMARY);
        g_signal_connect(drag, "pressed", G_CALLBACK(on_drag_begin), window);
        gtk_widget_add_controller(web_view, GTK_EVENT_CONTROLLER(drag));
    }

    gtk_window_set_child(GTK_WINDOW(window), web_view);
    gtk_window_present(GTK_WINDOW(window));
}

static void activate(GtkApplication* app, gpointer user_data)
{
    // Ensure GTK toplevel backgrounds are fully transparent via CSS
    hb_install_transparent_css();
    // Try to load ~/.hudbox.json
    const gchar* home = g_get_home_dir();
    gchar* path = home ? g_build_filename(home, ".hudbox.json", NULL) : NULL;

    GPtrArray* cfgs = NULL;
    if (path)
    {
        cfgs = hb_load_configs_from_json(path);
        g_free(path);
    }

    if (cfgs && cfgs->len > 0)
    {
        for (guint i = 0; i < cfgs->len; i++)
        {
            HbWindowCfg* cfg = (HbWindowCfg*)g_ptr_array_index(cfgs, i);
            hb_create_window(app, cfg);
            hb_window_cfg_clear(cfg);
            // free container struct itself; strings already cleared
            // freed by array free func, but we already clear strings to avoid leaks
        }
        g_ptr_array_free(cfgs, TRUE);
        return;
    }

    // Fallback: single default window if no config present/parsed
    HbWindowCfg def;
    hb_window_cfg_init_defaults(&def);
    hb_create_window(app, &def);
    hb_window_cfg_clear(&def);
}

int main(int argc, char** argv)
{
    GtkApplication* app = gtk_application_new("me.stegall.hudbox", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
