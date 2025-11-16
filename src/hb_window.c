#include "hb_window.h"
#include <webkit/webkit.h>

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

void hb_create_window(GtkApplication* app, const HbWindowCfg* cfg)
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
