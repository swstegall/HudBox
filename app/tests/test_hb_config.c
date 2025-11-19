#include <glib.h>
#include <glib/gstdio.h>
#include <unistd.h>
#include "../src/hb_config.h"

static void test_init_defaults(void)
{
    HbWindowCfg cfg = {0};
    hb_window_cfg_init_defaults(&cfg);

    g_assert_nonnull(cfg.title);
    g_assert_cmpstr(cfg.title, ==, "HudBox");
    g_assert_nonnull(cfg.address);
    g_assert_cmpstr(cfg.address, ==, "https://swstegall.github.io/HudBox/");
    g_assert_cmpint(cfg.width, ==, 800);
    g_assert_cmpint(cfg.height, ==, 600);
    g_assert_false(cfg.locked);
    g_assert_cmpfloat(cfg.opacity, ==, 0.9);
    g_assert_false(cfg.transparent);

    hb_window_cfg_clear(&cfg);
}

static void test_clear(void)
{
    HbWindowCfg cfg = {0};
    hb_window_cfg_init_defaults(&cfg);
    hb_window_cfg_clear(&cfg);
    g_assert_null(cfg.title);
    g_assert_null(cfg.address);
}

static gchar* write_temp_file(const gchar* contents)
{
    gchar* path = NULL;
    GError* err = NULL;

    // Create a temporary file in the g_get_tmp_dir()
    gchar* tmpl = g_build_filename(g_get_tmp_dir(), "hb_test_XXXXXX.json", NULL);
    int fd = g_mkstemp(tmpl);
    if (fd == -1)
    {
        g_free(tmpl);
        return NULL;
    }
    close(fd);

    if (!g_file_set_contents(tmpl, contents, -1, &err))
    {
        if (err) g_error_free(err);
        g_unlink(tmpl);
        g_free(tmpl);
        return NULL;
    }
    path = tmpl; // take ownership
    return path;
}

static void test_write_default_and_load(void)
{
    gchar* path = g_build_filename(g_get_tmp_dir(), "hb_default_test.json", NULL);
    g_unlink(path); // ensure clean

    gboolean ok = hb_write_default_config(path);
    g_assert_true(ok);

    GPtrArray* arr = hb_load_configs_from_json(path);
    g_assert_nonnull(arr);
    g_assert_cmpuint(arr->len, ==, 1);

    HbWindowCfg* cfg = (HbWindowCfg*)arr->pdata[0];
    g_assert_nonnull(cfg);
    g_assert_cmpstr(cfg->title, ==, "HudBox");

    // cleanup
    for (guint i = 0; i < arr->len; i++)
    {
        HbWindowCfg* c = arr->pdata[i];
        hb_window_cfg_clear(c);
        g_free(c);
    }
    g_ptr_array_free(arr, FALSE);
    g_unlink(path);
    g_free(path);
}

static void test_load_from_object_and_array(void)
{
    const gchar* obj_json =
        "{\n"
        "  \"title\": \"T\",\n"
        "  \"uri\": \"http://example.com\",\n"
        "  \"width\": 1024,\n"
        "  \"height\": 512,\n"
        "  \"locked\": true,\n"
        "  \"opacity\": 1.5,\n" // will be clamped to 1.0
        "  \"transparent\": true\n"
        "}\n";

    gchar* path1 = write_temp_file(obj_json);
    g_assert_nonnull(path1);

    GPtrArray* arr1 = hb_load_configs_from_json(path1);
    g_assert_nonnull(arr1);
    g_assert_cmpuint(arr1->len, ==, 1);

    HbWindowCfg* c1 = arr1->pdata[0];
    g_assert_cmpstr(c1->title, ==, "T");
    g_assert_cmpstr(c1->address, ==, "http://example.com");
    g_assert_cmpint(c1->width, ==, 1024);
    g_assert_cmpint(c1->height, ==, 512);
    g_assert_true(c1->locked);
    g_assert_cmpfloat(c1->opacity, ==, 1.0f);
    g_assert_true(c1->transparent);

    for (guint i = 0; i < arr1->len; i++)
    {
        HbWindowCfg* c = arr1->pdata[i];
        hb_window_cfg_clear(c);
        g_free(c);
    }
    g_ptr_array_free(arr1, FALSE);
    g_unlink(path1);
    g_free(path1);

    const gchar* arr_json =
        "[\n"
        "  { \"title\": \"A\", \"address\": \"a\", \"opacity\": -0.5 },\n"
        "  { \"title\": \"B\", \"address\": \"b\", \"opacity\": 0.5 }\n"
        "]\n";

    gchar* path2 = write_temp_file(arr_json);
    g_assert_nonnull(path2);

    GPtrArray* arr2 = hb_load_configs_from_json(path2);
    g_assert_nonnull(arr2);
    g_assert_cmpuint(arr2->len, ==, 2);

    HbWindowCfg* cA = arr2->pdata[0];
    HbWindowCfg* cB = arr2->pdata[1];
    g_assert_cmpfloat(cA->opacity, ==, 0.0f); // clamped
    g_assert_cmpfloat(cB->opacity, ==, 0.5f);

    for (guint i = 0; i < arr2->len; i++)
    {
        HbWindowCfg* c = arr2->pdata[i];
        hb_window_cfg_clear(c);
        g_free(c);
    }
    g_ptr_array_free(arr2, FALSE);
    g_unlink(path2);
    g_free(path2);
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/hb_config/init_defaults", test_init_defaults);
    g_test_add_func("/hb_config/clear", test_clear);
    g_test_add_func("/hb_config/write_default_and_load", test_write_default_and_load);
    g_test_add_func("/hb_config/load_from_object_and_array", test_load_from_object_and_array);

    return g_test_run();
}
