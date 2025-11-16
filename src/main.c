#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>

#include "hb_css.h"
#include "hb_config.h"
#include "hb_window.h"

static void activate(GtkApplication* app, gpointer user_data)
{
    // Ensure GTK toplevel backgrounds are fully transparent via CSS
    hb_install_transparent_css();

    // If a config path was supplied on the command line, it is stored on the app object
    const gchar* cmd_cfg_path = (const gchar*)g_object_get_data(G_OBJECT(app), "hb-config-path");

    GPtrArray* cfgs = NULL;

    if (cmd_cfg_path && *cmd_cfg_path)
    {
        // Load ONLY from the provided path; do not create a default file here
        cfgs = hb_load_configs_from_json(cmd_cfg_path);
    }
    else
    {
        // Try to load ~/.hudbox.json (create a default one if it doesn't exist)
        const gchar* home = g_get_home_dir();
        gchar* path = home ? g_build_filename(home, ".hudbox.json", NULL) : NULL;
        if (path)
        {
            hb_ensure_default_config_exists(path);
            cfgs = hb_load_configs_from_json(path);
            g_free(path);
        }
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

// Handle command-line arguments to accept an optional config path without triggering
// GApplication's default "open files" handling.
static int on_command_line(GApplication* gapp, GApplicationCommandLine* cmdline, gpointer user_data)
{
    GtkApplication* app = GTK_APPLICATION(gapp);

    int argc = 0;
    char** argv = g_application_command_line_get_arguments(cmdline, &argc);

    const char* cfg_path = NULL;
    if (argc > 1 && argv[1] && argv[1][0] != '-')
    {
        cfg_path = argv[1];
    }

    if (cfg_path)
    {
        g_object_set_data_full(G_OBJECT(app), "hb-config-path", g_strdup(cfg_path), g_free);
    }

    // Activate the application; activate() will read hb-config-path and proceed
    g_application_activate(gapp);

    g_strfreev(argv);
    // Return 0 to indicate successful handling of the command line
    return 0;
}

int main(int argc, char** argv)
{
    GtkApplication* app = gtk_application_new(
        "me.stegall.hudbox",
        G_APPLICATION_DEFAULT_FLAGS | G_APPLICATION_HANDLES_COMMAND_LINE);

    // Handle command-line to accept optional config path without triggering file-open errors
    g_signal_connect(app, "command-line", G_CALLBACK(on_command_line), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
