#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <glib.h>
#include <stdlib.h>

static gchar* hb_get_uri_from_config(void) {
    const gchar *home = g_get_home_dir();
    if (!home) return NULL;
    gchar *path = g_build_filename(home, ".hudbox.json", NULL);
    gchar *contents = NULL;
    gsize length = 0;
    GError *error = NULL;
    if (!g_file_get_contents(path, &contents, &length, &error)) {
        if (error) g_error_free(error);
        g_free(path);
        return NULL;
    }
    g_free(path);

    // Try to extract an address from either array/object forms using a simple regex.
    // Preferred new schema example:
    // [ { "address": "http://..." } ]
    GMatchInfo *match_info = NULL;
    gchar *uri = NULL;

    // 1) New schema: look for "address":"..."
    GRegex *regex = g_regex_new("\"address\"\\s*:\\s*\"([^\"]*)\"", G_REGEX_MULTILINE | G_REGEX_RAW, 0, NULL);
    if (regex && g_regex_match(regex, contents, 0, &match_info)) {
        uri = g_match_info_fetch(match_info, 1);
    }
    if (match_info) { g_match_info_free(match_info); match_info = NULL; }
    if (regex) { g_regex_unref(regex); regex = NULL; }

    g_free(contents);

    // Return duplicated string or NULL
    return uri; // newly allocated by g_match_info_fetch or NULL
}

// Get optional window title from config. Accepts quoted or unquoted key: title: "..."
static gchar* hb_get_title_from_config(void) {
    const gchar *home = g_get_home_dir();
    if (!home) return NULL;
    gchar *path = g_build_filename(home, ".hudbox.json", NULL);
    gchar *contents = NULL;
    gsize length = 0;
    GError *error = NULL;
    if (!g_file_get_contents(path, &contents, &length, &error)) {
        if (error) g_error_free(error);
        g_free(path);
        return NULL;
    }
    g_free(path);

    // Match title key (quoted or unquoted) followed by a quoted string value
    GMatchInfo *match_info = NULL;
    gchar *title = NULL;
    GRegex *regex = g_regex_new("(?:\\\"title\\\"|title)\\s*:\\s*\\\"([^\\\"]*)\\\"", G_REGEX_MULTILINE | G_REGEX_RAW, 0, NULL);
    if (regex && g_regex_match(regex, contents, 0, &match_info)) {
        title = g_match_info_fetch(match_info, 1);
    }
    if (match_info) { g_match_info_free(match_info); match_info = NULL; }
    if (regex) { g_regex_unref(regex); regex = NULL; }

    g_free(contents);
    return title; // newly allocated or NULL
}

// Get integer value (e.g., width/height) from config by key; accepts quoted or unquoted key
static gboolean hb_get_int_from_config(const gchar *key, gint *out_value) {
    if (!key || !out_value) return FALSE;
    const gchar *home = g_get_home_dir();
    if (!home) return FALSE;
    gchar *path = g_build_filename(home, ".hudbox.json", NULL);
    gchar *contents = NULL;
    gsize length = 0;
    GError *error = NULL;
    if (!g_file_get_contents(path, &contents, &length, &error)) {
        if (error) g_error_free(error);
        g_free(path);
        return FALSE;
    }
    g_free(path);

    // Build regex for key: ("key"|key)\s*:\s*(number)
    gchar *pattern = g_strdup_printf("(?:\\\"%s\\\"|%s)\\s*:\\s*([0-9]+)", key, key);
    GMatchInfo *match_info = NULL;
    GRegex *regex = g_regex_new(pattern, G_REGEX_MULTILINE | G_REGEX_RAW, 0, NULL);
    g_free(pattern);

    gboolean ok = FALSE;
    if (regex && g_regex_match(regex, contents, 0, &match_info)) {
        gchar *numstr = g_match_info_fetch(match_info, 1);
        if (numstr) {
            gchar *endp = NULL;
            long val = strtol(numstr, &endp, 10);
            if (endp && *endp == '\0') {
                *out_value = (gint)val;
                ok = TRUE;
            }
            g_free(numstr);
        }
    }
    if (match_info) { g_match_info_free(match_info); match_info = NULL; }
    if (regex) { g_regex_unref(regex); regex = NULL; }

    g_free(contents);
    return ok;
}

// Called when user starts dragging
static void on_drag_begin(GtkGestureClick *gesture,
                          gint n_press,
                          gdouble x,
                          gdouble y,
                          gpointer user_data) {
    GtkWindow *window = GTK_WINDOW(user_data);

    // GTK 4 replacement: get seat/device and start a system move
    GdkSurface *surface = gtk_native_get_surface(gtk_widget_get_native(GTK_WIDGET(window)));
    GdkDevice *device = gtk_gesture_get_device(GTK_GESTURE(gesture));
    if (surface && device) {
        gdk_toplevel_begin_move(GDK_TOPLEVEL(surface), device, n_press, x, y, GDK_CURRENT_TIME);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);

    // Defaults
    const gchar *default_title = "Borderless Draggable WebView";
    int width = 800;
    int height = 325;
    bool locked = false;

    // Load overrides from ~/.hudbox.json
    gchar *cfg_title = hb_get_title_from_config();
    hb_get_int_from_config("width", &width);
    hb_get_int_from_config("height", &height);

    gtk_window_set_title(GTK_WINDOW(window), cfg_title ? cfg_title : default_title);
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
    if (cfg_title) g_free(cfg_title);

    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    // Make the entire window (including all children) 80% opaque.
    // This is the correct, compositor-friendly way in GTK4 for whole-window translucency.
    gtk_widget_set_opacity(window, 1.0);

    // WebView
    GtkWidget *web_view = webkit_web_view_new();

    // Make WebView background transparent so window background shows through
    GdkRGBA transparent = (GdkRGBA){0, 0, 0, 0};
    webkit_web_view_set_background_color(WEBKIT_WEB_VIEW(web_view), &transparent);

    const gchar *default_uri = "https://www.google.com";
    gchar *cfg_uri = hb_get_uri_from_config();
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), cfg_uri ? cfg_uri : default_uri);
    if (cfg_uri) g_free(cfg_uri);

    // Drag-to-move
    if (!locked)
    {
        GtkGesture *drag = gtk_gesture_click_new();
        gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(drag), GTK_PHASE_BUBBLE);
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(drag), GDK_BUTTON_PRIMARY);
        g_signal_connect(drag, "pressed", G_CALLBACK(on_drag_begin), window);
        gtk_widget_add_controller(web_view, GTK_EVENT_CONTROLLER(drag));
    }

    gtk_window_set_child(GTK_WINDOW(window), web_view);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.example.hudbox", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
