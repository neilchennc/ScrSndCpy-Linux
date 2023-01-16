#include <gtk/gtk.h>
#include <gio/gio.h>
#include <math.h>
#include "log.h"
#include "net.h"
#include "popen.h"
#include "preference.h"
#include "track_device.h"

GtkWidget *g_device_list_box;
GtkWidget *g_device_entry;
GtkWidget *g_borderless_check_button;
GtkWidget *g_always_on_top_check_button;
GtkWidget *g_fullscreen_check_button;
GtkWidget *g_no_control_check_button;
GtkWidget *g_stay_awake_check_button;
GtkWidget *g_turn_screen_off_check_button;
GtkWidget *g_no_power_on_check_button;
GtkWidget *g_power_off_on_close_check_button;
GtkWidget *g_show_touches_check_button;
GtkWidget *g_no_key_repeat_check_button;
GtkWidget *g_max_size_entry;
GtkWidget *g_bitrate_entry;
GtkWidget *g_max_fps_entry;
GtkWidget *g_play_button;
GtkWidget *g_message_scrolled_window;
GtkWidget *g_message_text_view;

gboolean append_message_text_view(gpointer arg)
{
    const char *str = arg;
    GtkTextIter iter;
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_message_text_view));
    gtk_text_buffer_get_end_iter(text_buffer, &iter);
    gtk_text_buffer_insert(text_buffer, &iter, str, strlen(str));

    while (gtk_events_pending())
        gtk_main_iteration();

    GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(g_message_scrolled_window));
    gdouble upper = gtk_adjustment_get_upper(vadj);
    gtk_adjustment_set_value(vadj, upper);

    free(arg);
    return FALSE;
}

void append_message_text_view_new(const char *message)
{
    char *str = calloc(strlen(message) + 1, sizeof(char));
    memcpy(str, message, strlen(message));
    g_idle_add((GSourceFunc)append_message_text_view, str);
}

gboolean start_adb_server_and_connect(gpointer arg)
{
    track_device_close();
    popen_run("adb start-server", append_message_text_view_new);
    track_device_start();
    return FALSE;
}

gboolean launch_scrcpy(gpointer arg)
{
    char *serial = arg;
    char message[512];
    char command[256];
    const char *ptr;

    // device serial
    sprintf(command, "scrcpy -s %s", serial);
    // max size
    ptr = gtk_entry_get_text(GTK_ENTRY(g_max_size_entry));
    if (strlen(ptr) > 0)
    {
        strcat(command, " --max-size=");
        strcat(command, ptr);
    }
    // bitrate
    ptr = gtk_entry_get_text(GTK_ENTRY(g_bitrate_entry));
    if (strlen(ptr) > 0)
    {
        strcat(command, " --bit-rate=");
        strcat(command, ptr);
        strcat(command, "M");
    }
    // max fps
    ptr = gtk_entry_get_text(GTK_ENTRY(g_max_fps_entry));
    if (strlen(ptr))
    {
        strcat(command, " --max-fps=");
        strcat(command, ptr);
    }
    // borderless
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_borderless_check_button)))
        strcat(command, " --window-borderless");
    // always on top
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_always_on_top_check_button)))
        strcat(command, " --always-on-top");
    // fullscreen
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_fullscreen_check_button)))
        strcat(command, " --fullscreen");
    // no control
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_no_control_check_button)))
        strcat(command, " --no-control");
    // stay awake
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_stay_awake_check_button)))
        strcat(command, " --stay-awake");
    // turn screen off
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_turn_screen_off_check_button)))
        strcat(command, " --turn-screen-off");
    // Don't power on device on start
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_no_power_on_check_button)))
        strcat(command, " --no-power-on");
    // power off on close
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_power_off_on_close_check_button)))
        strcat(command, " --power-off-on-close");
    // show touches
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_show_touches_check_button)))
        strcat(command, " --show-touches");
    // no key repeat
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_no_key_repeat_check_button)))
        strcat(command, " --no-key-repeat");

    sprintf(message, "Run command: %s\n", command);
    append_message_text_view_new(message);

    popen_run(command, append_message_text_view_new);

    return FALSE;
}

gboolean launch_sndcpy(gpointer arg)
{
    char *serial = arg;
    char message[128];
    char command[64];

    sprintf(command, "./sndcpy %s", serial);
    sprintf(message, "Run command: %s\n", command);
    append_message_text_view_new(message);

    popen_run(command, append_message_text_view_new);

    return FALSE;
}

void adb_stop_sndcpy(char *serial)
{
    char command[128];

    sprintf(command, "adb -s %s shell am force-stop com.rom1v.sndcpy", serial);
    popen_run(command, NULL);
}

gboolean connect_to_device(gpointer arg)
{
    char *serial = arg;
    char buffer[256];
    int exit_code;

    n_print("Connect to %s...\n", serial);

    // Check if valid address format
    if (strlen(serial) < 32)
    {
        char ip[32] = {0};
        sscanf(serial, "%[^:\r\t\n]", ip);
        if (is_valid_ip_address(ip))
        {
            // display message
            sprintf(buffer, "Connecting to %s...\n", ip);
            append_message_text_view_new(buffer);

            // connect to it via adb
            sprintf(buffer, "adb connect %s", ip);
            popen_run(buffer, append_message_text_view_new);
        }
    }

    // Check device state
    sprintf(buffer, "adb -s %s get-state", serial);
    exit_code = popen_run(buffer, NULL);

    if (exit_code != 0)
    {
        n_print("Device state is not ready, exit code: %d\n", exit_code);
        append_message_text_view_new("Device state is not ready.\nPlease check for a confirmation dialog on your device.\n");
        free(serial);
        return FALSE;
    }

    // launch scrcpy & sndcpy
    GThread *scrcpy_thread = g_thread_new("launch-scrcpy", (GThreadFunc)launch_scrcpy, serial);
    GThread *sndcpy_thread = g_thread_new("launch-sndcpy", (GThreadFunc)launch_sndcpy, serial);

    g_thread_join(scrcpy_thread);
    append_message_text_view_new("scrcpy stopped.\n");

    adb_stop_sndcpy(serial);

    g_thread_join(sndcpy_thread);
    append_message_text_view_new("sndcpy stopped.\n");

    n_print("Exit connect to %s.\n", serial);
    free(serial);
    return FALSE;
}

/**
 * @brief Called when new device list is coming.
 */
gboolean on_track_device_new_devices(gpointer arg)
{
    adb_device_t *adb_device = arg;

    n_print("new devices: size: %d\n", adb_device->device_size);

    // Clear list
    GtkListBoxRow *list_box_row;
    while ((list_box_row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(g_device_list_box), 0)))
    {
        gtk_container_remove(GTK_CONTAINER(g_device_list_box), GTK_WIDGET(list_box_row));
    }

    // Insert new devices
    for (int i = 0; i < adb_device->device_size; i++)
    {
        n_print("new device: %s\n", adb_device->name[i]);
        GtkWidget *label = gtk_label_new(adb_device->name[i]);
        // PangoAttrList *attr_list = pango_attr_list_new();
        // PangoFontDescription *desc = pango_font_description_new();
        // pango_font_description_set_size(desc, 24 * PANGO_SCALE);
        // PangoAttribute *attr = pango_attr_font_desc_new(desc);
        // pango_attr_list_insert(attr_list, attr);
        // gtk_label_set_attributes(GTK_LABEL(label), attr_list);
        gtk_label_set_xalign(GTK_LABEL(label), 0.0); // align left
        gtk_list_box_insert(GTK_LIST_BOX(g_device_list_box), label, i);
        gtk_widget_show(label);
    }

    free(adb_device);

    return FALSE;
}

/**
 * @brief Called when disconnected with adbd.
 */
gboolean on_track_device_disconnected(gpointer data)
{
    n_print("adbd disconnected\n");

    // Clear list
    GtkListBoxRow *list_box_row;
    while ((list_box_row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(g_device_list_box), 0)))
    {
        gtk_container_remove(GTK_CONTAINER(g_device_list_box), GTK_WIDGET(list_box_row));
    }

    append_message_text_view_new("Connection lost, restart daemon now...\n");

    g_thread_new("start-adb-server", (GThreadFunc)start_adb_server_and_connect, NULL);

    return FALSE;
}

void set_default_size_and_fps()
{
    GdkDisplay *default_display = gdk_display_get_default();
    int n_monitor = gdk_display_get_n_monitors(default_display);
    // GdkMonitor *monitor = gdk_display_get_primary_monitor(default_display);
    GdkMonitor *monitor = NULL;
    GdkRectangle rect = {0};
    double refresh_rate = 0;
    double max_refresh_rate = 0;
    int max_size = 0;
    char temp[16];

    for (int i = 0; i < n_monitor; i++)
    {
        monitor = gdk_display_get_monitor(default_display, i);
        gdk_monitor_get_geometry(monitor, &rect);
        refresh_rate = gdk_monitor_get_refresh_rate(monitor) / 1000.0;
        n_print("Got monitor [%d]: width: %d, height: %d, refresh rate: %.3f\n", i, rect.width, rect.height, refresh_rate);

        max_size = MAX(MAX(max_size, rect.width), rect.height);
        max_refresh_rate = MAX(refresh_rate, max_refresh_rate);
    }

    n_print("max monitor size: %d, max refresh rate: %.3f\n", max_size, max_refresh_rate);

    if (max_size > 0)
    {
        sprintf(temp, "%d", max_size);
        gtk_entry_set_text(GTK_ENTRY(g_max_size_entry), temp);
    }
    if (max_refresh_rate > 0)
    {
        sprintf(temp, "%d", (int)round(max_refresh_rate));
        gtk_entry_set_text(GTK_ENTRY(g_max_fps_entry), temp);
    }
}

void load_from_preference_file()
{
    char temp[16];
    preference_t pref = {0};
    preference_load(&pref);

    if (pref.max_size > 0)
    {
        sprintf(temp, "%d", pref.max_size);
        gtk_entry_set_text(GTK_ENTRY(g_max_size_entry), temp);
    }
    if (pref.bitrate > 0)
    {
        sprintf(temp, "%d", pref.bitrate);
        gtk_entry_set_text(GTK_ENTRY(g_bitrate_entry), temp);
    }
    if (pref.max_fps > 0)
    {
        sprintf(temp, "%d", pref.max_fps);
        gtk_entry_set_text(GTK_ENTRY(g_max_fps_entry), temp);
    }

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_borderless_check_button), pref.borderless != 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_always_on_top_check_button), pref.always_on_top != 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_fullscreen_check_button), pref.fullscreen != 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_no_control_check_button), pref.no_control != 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_stay_awake_check_button), pref.stay_awake != 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_turn_screen_off_check_button), pref.turn_screen_off != 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_no_power_on_check_button), pref.no_power_on != 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_power_off_on_close_check_button), pref.power_off_on_close != 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_show_touches_check_button), pref.show_touches != 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_no_key_repeat_check_button), pref.no_key_repeat != 0);
}

void save_to_preference_file()
{
    const gchar *nptr;
    preference_t pref = {0};

    nptr = gtk_entry_get_text(GTK_ENTRY(g_max_size_entry));
    if (strlen(nptr) > 0)
    {
        pref.max_size = g_ascii_strtoll(nptr, NULL, 10);
    }

    nptr = gtk_entry_get_text(GTK_ENTRY(g_bitrate_entry));
    if (strlen(nptr) > 0)
    {
        pref.bitrate = g_ascii_strtoll(nptr, NULL, 10);
    }

    nptr = gtk_entry_get_text(GTK_ENTRY(g_max_fps_entry));
    if (strlen(nptr) > 0)
    {
        pref.max_fps = g_ascii_strtoll(nptr, NULL, 10);
    }

    pref.borderless = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_borderless_check_button)) ? 1 : 0;
    pref.always_on_top = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_always_on_top_check_button)) ? 1 : 0;
    pref.fullscreen = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_fullscreen_check_button)) ? 1 : 0;
    pref.no_control = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_no_control_check_button)) ? 1 : 0;
    pref.stay_awake = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_stay_awake_check_button)) ? 1 : 0;
    pref.turn_screen_off = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_turn_screen_off_check_button)) ? 1 : 0;
    pref.no_power_on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_no_power_on_check_button)) ? 1 : 0;
    pref.power_off_on_close = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_power_off_on_close_check_button)) ? 1 : 0;
    pref.show_touches = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_show_touches_check_button)) ? 1 : 0;
    pref.no_key_repeat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_no_key_repeat_check_button)) ? 1 : 0;

    preference_save(&pref);
}

void check_and_show_version_info()
{
    append_message_text_view_new("ScrSndCpy 1.1 <https://github.com/neilchennc/ScrSndCpy-Linux>\n");
    append_message_text_view_new("sndcpy 1.1 <https://github.com/rom1v/sndcpy>\n");
    if (popen_run("scrcpy -v", append_message_text_view_new) == 0)
    {
        // start tracking devices
        track_device_init(on_track_device_new_devices, on_track_device_disconnected);
        track_device_start();
    }
    else
    {
        // show message and disable button
        append_message_text_view_new(
            "Failed to check scrcpy version.\n"
            "Execute following command to install scrcpy:\n"
            "sudo apt install scrcpy\n");
        gtk_widget_set_sensitive(g_play_button, FALSE);
    }
}

gboolean test_message(gpointer arg)
{
    char *str = arg;
    while (1)
    {
        append_message_text_view_new(str);
        // g_usleep(g_random_int_range(50000, 50005));
        g_usleep(50000);
    }

    return FALSE;
}

int main(int argc, char *argv[])
{
    GtkBuilder *builder;
    GtkWidget *window;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "main.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    gtk_builder_connect_signals(builder, NULL);

    // export widgets to global
    g_device_list_box = GTK_WIDGET(gtk_builder_get_object(builder, "list_box_device"));
    g_device_entry = GTK_WIDGET(gtk_builder_get_object(builder, "entry_serial"));
    g_borderless_check_button = GTK_WIDGET(gtk_builder_get_object(builder, "check_button_borderless"));
    g_always_on_top_check_button = GTK_WIDGET(gtk_builder_get_object(builder, "check_button_always_on_top"));
    g_fullscreen_check_button = GTK_WIDGET(gtk_builder_get_object(builder, "check_button_fullscreen"));
    g_no_control_check_button = GTK_WIDGET(gtk_builder_get_object(builder, "check_button_no_control"));
    g_stay_awake_check_button = GTK_WIDGET(gtk_builder_get_object(builder, "check_button_stay_awake"));
    g_turn_screen_off_check_button = GTK_WIDGET(gtk_builder_get_object(builder, "check_button_turn_screen_off"));
    g_no_power_on_check_button = GTK_WIDGET(gtk_builder_get_object(builder, "check_button_no_power_on"));
    g_power_off_on_close_check_button = GTK_WIDGET(gtk_builder_get_object(builder, "check_button_power_off_on_close"));
    g_show_touches_check_button = GTK_WIDGET(gtk_builder_get_object(builder, "check_buttun_show_touches"));
    g_no_key_repeat_check_button = GTK_WIDGET(gtk_builder_get_object(builder, "check_button_no_key_repeat"));
    g_max_size_entry = GTK_WIDGET(gtk_builder_get_object(builder, "entry_max_size"));
    g_bitrate_entry = GTK_WIDGET(gtk_builder_get_object(builder, "entry_bitrate"));
    g_max_fps_entry = GTK_WIDGET(gtk_builder_get_object(builder, "entry_max_fps"));
    g_play_button = GTK_WIDGET(gtk_builder_get_object(builder, "button_play"));
    g_message_text_view = GTK_WIDGET(gtk_builder_get_object(builder, "text_view_message"));
    g_message_scrolled_window = GTK_WIDGET(gtk_builder_get_object(builder, "scrolled_window_message"));

    g_object_unref(builder);

    gtk_widget_show(window);

    // Set default size and fps for scrcpy
    set_default_size_and_fps();

    // Load preference
    load_from_preference_file();

    // check scrcpy and show version info
    check_and_show_version_info();

    gtk_main();

    return 0;
}

void on_list_box_device_selected_rows_changed()
{
    GtkListBoxRow *selected_row = gtk_list_box_get_selected_row(GTK_LIST_BOX(g_device_list_box));
    if (selected_row != NULL)
    {
        GList *list = gtk_container_get_children(GTK_CONTAINER(selected_row));
        const char *str = gtk_label_get_text(GTK_LABEL(list->data));
        n_print("Selected item: %s\n", str);
        gtk_entry_set_text(GTK_ENTRY(g_device_entry), str);
    }
}

void on_play_button_clicked()
{
    const char *serial = gtk_entry_get_text(GTK_ENTRY(g_device_entry));

    if (strlen(serial) > 0)
    {
        char *serial_new = calloc(strlen(serial) + 1, sizeof(char));
        memcpy(serial_new, serial, strlen(serial));
        g_thread_new("connect-to-device", (GThreadFunc)connect_to_device, serial_new);
    }
}

void on_main_destroy()
{
    // Save current preferences
    save_to_preference_file();

    gtk_main_quit();
}