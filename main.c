#include <gtk/gtk.h>
#include <webkit/webkit.h>

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
    gtk_window_set_title(GTK_WINDOW(window), "Borderless Draggable WebView");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    // Make the entire window (including all children) 80% opaque.
    // This is the correct, compositor-friendly way in GTK4 for whole-window translucency.
    gtk_widget_set_opacity(window, 0.8);

    // WebView
    GtkWidget *web_view = webkit_web_view_new();

    // Make WebView background transparent so window background shows through
    GdkRGBA transparent = {0, 0, 0, 0};
    webkit_web_view_set_background_color(WEBKIT_WEB_VIEW(web_view), &transparent);

    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), "https://www.example.com");

    // Drag-to-move
    GtkGesture *drag = gtk_gesture_click_new();
    gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(drag), GTK_PHASE_BUBBLE);
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(drag), GDK_BUTTON_PRIMARY);
    g_signal_connect(drag, "pressed", G_CALLBACK(on_drag_begin), window);
    gtk_widget_add_controller(web_view, GTK_EVENT_CONTROLLER(drag));

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
