#include <gtk/gtk.h>

void hb_install_transparent_css(void)
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
