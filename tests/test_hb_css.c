#include <glib.h>
#include <gtk/gtk.h>
#include "../src/hb_css.h"

static void test_install_transparent_css_smoke(void)
{
    // Initialize GTK; skip test if no display is available (e.g., headless CI)
    if (!gtk_init_check())
    {
        g_test_skip("No GTK display available; skipping hb_css smoke test");
        return;
    }

    // Should be safe to call multiple times
    hb_install_transparent_css();
    hb_install_transparent_css();

    // Create a widget and add the class to ensure no warnings/errors are produced.
    GtkWidget* w = gtk_window_new();
    gtk_widget_add_css_class(w, "hb-transparent");
    g_assert_true(gtk_widget_has_css_class(w, "hb-transparent"));

    // Cleanup
    gtk_window_close(GTK_WINDOW(w));
    g_object_unref(w);
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/hb_css/install_transparent_css_smoke", test_install_transparent_css_smoke);

    return g_test_run();
}
