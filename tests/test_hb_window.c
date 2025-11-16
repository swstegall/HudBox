#include <glib.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include "../src/hb_window.h"
#include "../src/hb_css.h"

static gboolean should_run_webkit_tests(void)
{
    const char* flag = g_getenv("HB_ENABLE_WEBKIT_TESTS");
    return flag && flag[0] != '\0';
}

static gboolean register_or_skip(GtkApplication* app, const char* why)
{
    GError* error = NULL;
    if (!g_application_register(G_APPLICATION(app), NULL, &error))
    {
        g_test_skip(why);
        if (error) g_error_free(error);
        return FALSE;
    }
    return TRUE;
}

static HbWindowCfg make_cfg(const char* title, const char* addr)
{
    HbWindowCfg cfg = {0};
    hb_window_cfg_init_defaults(&cfg);
    if (title) { g_free(cfg.title); cfg.title = g_strdup(title); }
    if (addr) { g_free(cfg.address); cfg.address = g_strdup(addr); }
    return cfg;
}

static void clear_cfg(HbWindowCfg* cfg)
{
    hb_window_cfg_clear(cfg);
}

static void on_activate_create(GtkApplication* app, gpointer user_data)
{
    HbWindowCfg* cfg = (HbWindowCfg*)user_data;
    hb_create_window(app, cfg);
}

static void close_all_windows(GtkApplication* app)
{
    GList* wins = gtk_application_get_windows(app);
    for (GList* l = wins; l != NULL; l = l->next)
    {
        GtkWindow* win = GTK_WINDOW(l->data);
        if (win)
        {
            gtk_window_close(win);
        }
    }
    // Iterate the main context a few times to process close events
    for (int i = 0; i < 3; i++)
    {
        while (g_main_context_iteration(NULL, FALSE)) {}
    }
}

static void test_create_window_smoke_default(void)
{
    if (!gtk_init_check() || !should_run_webkit_tests())
    {
        g_test_skip("Prereqs not met (display or HB_ENABLE_WEBKIT_TESTS not set); skipping");
        return;
    }

    GtkApplication* app = gtk_application_new("com.hudbox.tests.window.default", G_APPLICATION_NON_UNIQUE);
    HbWindowCfg cfg = make_cfg("T", "https://example.com");

    if (!register_or_skip(app, "GApplication could not register; skipping"))
    {
        clear_cfg(&cfg);
        g_object_unref(app);
        return;
    }

    // Create window on activate so startup is emitted correctly
    g_signal_connect(app, "activate", G_CALLBACK(on_activate_create), &cfg);
    g_application_activate(G_APPLICATION(app));

    // Cleanup
    close_all_windows(app);
    clear_cfg(&cfg);
    g_object_unref(app);
}

static void test_create_window_smoke_transparent_and_locked(void)
{
    if (!gtk_init_check() || !should_run_webkit_tests())
    {
        g_test_skip("Prereqs not met (display or HB_ENABLE_WEBKIT_TESTS not set); skipping");
        return;
    }

    // Ensure CSS is installed so the class is recognized
    hb_install_transparent_css();

    GtkApplication* app = gtk_application_new("com.hudbox.tests.window.transparent", G_APPLICATION_NON_UNIQUE);
    HbWindowCfg cfg = make_cfg("T2", "https://example.com");
    cfg.transparent = TRUE;
    cfg.locked = TRUE; // ensure gesture isn't added, but still should not crash

    if (!register_or_skip(app, "GApplication could not register; skipping"))
    {
        clear_cfg(&cfg);
        g_object_unref(app);
        return;
    }

    g_signal_connect(app, "activate", G_CALLBACK(on_activate_create), &cfg);
    g_application_activate(G_APPLICATION(app));

    close_all_windows(app);
    clear_cfg(&cfg);
    g_object_unref(app);
}

static void test_create_window_smoke_opacity_clamp(void)
{
    if (!gtk_init_check() || !should_run_webkit_tests())
    {
        g_test_skip("Prereqs not met (display or HB_ENABLE_WEBKIT_TESTS not set); skipping");
        return;
    }

    GtkApplication* app = gtk_application_new("com.hudbox.tests.window.opacity", G_APPLICATION_NON_UNIQUE);

    if (!register_or_skip(app, "GApplication could not register; skipping"))
    {
        g_object_unref(app);
        return;
    }

    HbWindowCfg cfg_low = make_cfg("Low", "https://example.com");
    cfg_low.opacity = -5.0; // should clamp to 0.0 internally
    g_signal_connect(app, "activate", G_CALLBACK(on_activate_create), &cfg_low);
    g_application_activate(G_APPLICATION(app));
    g_signal_handlers_disconnect_by_func(app, on_activate_create, &cfg_low);
    clear_cfg(&cfg_low);

    HbWindowCfg cfg_high = make_cfg("High", "https://example.com");
    cfg_high.opacity = 5.0; // should clamp to 1.0 internally
    g_signal_connect(app, "activate", G_CALLBACK(on_activate_create), &cfg_high);
    g_application_activate(G_APPLICATION(app));
    g_signal_handlers_disconnect_by_func(app, on_activate_create, &cfg_high);
    clear_cfg(&cfg_high);

    g_object_unref(app);
}

int main(int argc, char** argv)
{
    // Configure WebKit for test environment to avoid sandbox/bwrap issues
    g_setenv("WEBKIT_DISABLE_SANDBOX", "1", TRUE);
    g_setenv("WEBKIT_USE_SINGLE_WEB_PROCESS", "1", TRUE);

    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/hb_window/create_default", test_create_window_smoke_default);
    g_test_add_func("/hb_window/create_transparent_locked", test_create_window_smoke_transparent_and_locked);
    g_test_add_func("/hb_window/create_opacity_clamp", test_create_window_smoke_opacity_clamp);

    return g_test_run();
}
