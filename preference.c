#include "preference.h"

#include "log.h"
#include <glib.h>
#include <glib/gprintf.h>

#define PREFERENCE_FILENAME "ScrSndCpy.ini"
#define PREFERENCE_GROUP_NAME "ScrSndCpy"
#define PREFERENCE_KEY_MAX_SIZE "MaxSize"
#define PREFERENCE_KEY_BITRATE "Bitrate"
#define PREFERENCE_KEY_MAX_FPS "MaxFps"
#define PREFERENCE_KEY_BORDERLESS "Borderless"
#define PREFERENCE_KEY_ALWAYS_ON_TOP "AlwaysOnTop"
#define PREFERENCE_KEY_FULLSCREEN "Fullscreen"
#define PREFERENCE_KEY_NO_CONTROL "NoControl"
#define PREFERENCE_KEY_STAY_AWAKE "StayAwake"
#define PREFERENCE_KEY_TURN_SCREEN_OFF "TurnScreenOff"
#define PREFERENCE_KEY_NO_POWER_ON "NoPowerOn"
#define PREFERENCE_KEY_POWER_OFF_ON_CLOSE "PowerOffOnClose"
#define PREFERENCE_KEY_SHOW_TOUCHES "ShowTouches"
#define PREFERENCE_KEY_NO_KEY_REPEAT "NoKeyRepeat"

gint preference_read_integer(GKeyFile *key_file, const gchar *key, gint def_value)
{
    GError *error = NULL;
    gint value = g_key_file_get_integer(key_file, PREFERENCE_GROUP_NAME, key, &error);

    if (error)
    {
        n_warning("Failed to read value of the key: %s\n%s\n", key, error->message);
        g_error_free(error);
        return def_value;
    }
    else
    {
        n_debug("key: %s, value: %d\n", key, value);
        return value;
    }
}

int preference_load(preference_t *pref)
{
    GKeyFile *key_file;
    GError *error = NULL;
    // gint value;

    if (pref == NULL)
    {
        // Ignore
        return -1;
    }

    key_file = g_key_file_new();

    if (!g_key_file_load_from_file(key_file, PREFERENCE_FILENAME, G_KEY_FILE_NONE, &error))
    {
        if (error)
        {
            n_warning("Failed to read keyfile: %s", error->message);
            g_error_free(error);
        }
        g_key_file_free(key_file);
        return -1;
    }

    pref->max_size = preference_read_integer(key_file, PREFERENCE_KEY_MAX_SIZE, 0);
    pref->bitrate = preference_read_integer(key_file, PREFERENCE_KEY_BITRATE, 0);
    pref->max_fps = preference_read_integer(key_file, PREFERENCE_KEY_MAX_FPS, 0);
    pref->borderless = preference_read_integer(key_file, PREFERENCE_KEY_BORDERLESS, 0);
    pref->always_on_top = preference_read_integer(key_file, PREFERENCE_KEY_ALWAYS_ON_TOP, 0);
    pref->fullscreen = preference_read_integer(key_file, PREFERENCE_KEY_FULLSCREEN, 0);
    pref->no_control = preference_read_integer(key_file, PREFERENCE_KEY_NO_CONTROL, 0);
    pref->stay_awake = preference_read_integer(key_file, PREFERENCE_KEY_STAY_AWAKE, 0);
    pref->turn_screen_off = preference_read_integer(key_file, PREFERENCE_KEY_TURN_SCREEN_OFF, 0);
    pref->no_power_on = preference_read_integer(key_file, PREFERENCE_KEY_NO_POWER_ON, 0);
    pref->power_off_on_close = preference_read_integer(key_file, PREFERENCE_KEY_POWER_OFF_ON_CLOSE, 0);
    pref->show_touches = preference_read_integer(key_file, PREFERENCE_KEY_SHOW_TOUCHES, 0);
    pref->no_key_repeat = preference_read_integer(key_file, PREFERENCE_KEY_NO_KEY_REPEAT, 0);

    // Release
    g_key_file_free(key_file);

    if (error != NULL)
    {
        g_error_free(error);
    }

    return 0;

    // return 0;
}

void preference_save(preference_t *pref)
{
    GKeyFile *key_file;
    GError *error = NULL;
    // gint value;

    if (pref == NULL)
    {
        // Ignore
        return;
    }

    key_file = g_key_file_new();

    g_key_file_set_integer(key_file, PREFERENCE_GROUP_NAME, PREFERENCE_KEY_MAX_SIZE, pref->max_size);
    g_key_file_set_integer(key_file, PREFERENCE_GROUP_NAME, PREFERENCE_KEY_BITRATE, pref->bitrate);
    g_key_file_set_integer(key_file, PREFERENCE_GROUP_NAME, PREFERENCE_KEY_MAX_FPS, pref->max_fps);
    g_key_file_set_integer(key_file, PREFERENCE_GROUP_NAME, PREFERENCE_KEY_BORDERLESS, pref->borderless);
    g_key_file_set_integer(key_file, PREFERENCE_GROUP_NAME, PREFERENCE_KEY_ALWAYS_ON_TOP, pref->always_on_top);
    g_key_file_set_integer(key_file, PREFERENCE_GROUP_NAME, PREFERENCE_KEY_FULLSCREEN, pref->fullscreen);
    g_key_file_set_integer(key_file, PREFERENCE_GROUP_NAME, PREFERENCE_KEY_NO_CONTROL, pref->no_control);
    g_key_file_set_integer(key_file, PREFERENCE_GROUP_NAME, PREFERENCE_KEY_STAY_AWAKE, pref->stay_awake);
    g_key_file_set_integer(key_file, PREFERENCE_GROUP_NAME, PREFERENCE_KEY_TURN_SCREEN_OFF, pref->turn_screen_off);
    g_key_file_set_integer(key_file, PREFERENCE_GROUP_NAME, PREFERENCE_KEY_NO_POWER_ON, pref->no_power_on);
    g_key_file_set_integer(key_file, PREFERENCE_GROUP_NAME, PREFERENCE_KEY_POWER_OFF_ON_CLOSE, pref->power_off_on_close);
    g_key_file_set_integer(key_file, PREFERENCE_GROUP_NAME, PREFERENCE_KEY_SHOW_TOUCHES, pref->show_touches);
    g_key_file_set_integer(key_file, PREFERENCE_GROUP_NAME, PREFERENCE_KEY_NO_KEY_REPEAT, pref->no_key_repeat);

    // Save as a file
    if (!g_key_file_save_to_file(key_file, PREFERENCE_FILENAME, &error))
    {
        n_warning("Cannot save to preference: %s\n", error->message);
    }

    // Release
    g_key_file_free(key_file);

    if (error != NULL)
    {
        g_error_free(error);
    }
}