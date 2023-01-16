#ifndef PREFERENCE_H
#define PREFERENCE_H

typedef struct
{
    // Entry
    int max_size;
    int bitrate;
    int max_fps;

    // Checkbox
    int borderless;
    int always_on_top;
    int fullscreen;
    int no_control;
    int stay_awake;
    int turn_screen_off;
    int no_power_on;
    int power_off_on_close;
    int show_touches;
    int no_key_repeat;
} preference_t;

int preference_load(preference_t *pref);
void preference_save(preference_t *pref);

#endif // #ifndef PREFERENCE_H