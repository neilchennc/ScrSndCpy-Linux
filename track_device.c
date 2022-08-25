#include "track_device.h"
#include "log.h"
#include <gtk/gtk.h>
#include <gio/gio.h>

GSourceFunc g_new_device_callback;
GSourceFunc g_disconnected_callback;

GInputStream *input_stream;
GOutputStream *output_stream;
GSocketClient *client;
GSocketConnection *connection;

void parse_data(char *data, int size)
{
    adb_device_t *adb_device;
    char *payload_start, *payload_end;
    char *ptr;
    char prefix[8] = {0};
    unsigned long payload_length = 0;
    int index = 0;

    while (index + 4 <= size)
    {
        if (strncmp(&data[index], "OKAY", 4) == 0)
        {
            n_print("parse data: OKAY\n");
            index += 4;
            continue;
        }
        if (strncmp(&data[index], "0000", 4) == 0)
        {
            // notify empty device list
            adb_device = calloc(1, sizeof(adb_device_t));
            g_idle_add((GSourceFunc)g_new_device_callback, adb_device);

            n_print("parse data: empty device list\n");
            index += 4;
            continue;
        }

        adb_device = calloc(1, sizeof(adb_device_t));
        int device_index = 0;

        strncpy(prefix, &data[index], 4);
        payload_length = strtoul(prefix, NULL, 16);

        payload_start = &data[index + 4];
        payload_end = &data[index + 4 + payload_length];

        ptr = payload_start;
        while (ptr < payload_end)
        {
            char device_state[32]; // unused
            char *device_name = adb_device->name[device_index];
            int read_count;
            sscanf(ptr, "%[^\t]\t%[^\n]\n%n", device_name, device_state, &read_count);
            n_print("parse data: serial: %s, state: %s\n", device_name, device_state);
            device_index++;
            ptr += read_count;
        }

        adb_device->device_size = device_index;
        g_idle_add((GSourceFunc)g_new_device_callback, adb_device);

        index = index + 4 + payload_length;
    }
}

gboolean connect_to_adbd(gpointer arg)
{
    gssize read_size = 0;
    gchar buffer[1024] = {0};
    GError *error = NULL;

    n_print("Connecting to adbd...\n");

    client = g_socket_client_new();
    connection = g_socket_client_connect_to_host(client, "127.0.0.1", 5037, NULL, &error);

    if (error)
    {
        n_print("Cannot connect to adbd: %s\n", error->message);
        g_error_free(error);
        g_idle_add((GSourceFunc)g_disconnected_callback, NULL);
        return FALSE;
    }
    else
    {
        n_print("adbd connected.\n");
    }

    input_stream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    output_stream = g_io_stream_get_output_stream(G_IO_STREAM(connection));

    g_output_stream_write(output_stream, "0012host:track-devices", 22, NULL, &error);
    if (error)
    {
        n_print("Failed to write data: %s\n", error->message);
        g_error_free(error);
        g_idle_add((GSourceFunc)g_disconnected_callback, NULL);
        return FALSE;
    }

    while (TRUE)
    {
        read_size = g_input_stream_read(input_stream, buffer, sizeof(buffer), NULL, &error);
        if (read_size <= 0)
        {
            n_print("Disconnected.\n");
            g_idle_add((GSourceFunc)g_disconnected_callback, NULL);
            break;
        }
        if (error)
        {
            n_print("Failed to read data: %s\n", error->message);
            g_idle_add((GSourceFunc)g_disconnected_callback, NULL);
            break;
        }

        buffer[read_size] = '\0';
        n_print("adbd: read size: %ld, data: %s\n", read_size, buffer);
#if 0 // test only
        parse_data("OKAY", 4);
        parse_data("0000", 4);
        parse_data("OKAY0000", 8);
        parse_data("0008AAA\tBBB\n", 12);
        parse_data("0013AAA\tBBB\nCC CC\tDD D\n", 23);
#else
        parse_data(buffer, read_size);
#endif
    }

    if (error)
    {
        g_error_free(error);
    }

    return FALSE;
}

void track_device_init(GSourceFunc new_device, GSourceFunc disconnected)
{
    g_new_device_callback = new_device;
    g_disconnected_callback = disconnected;
}

void track_device_start()
{
    g_thread_new("adbd", (GThreadFunc)connect_to_adbd, NULL);
}

void track_device_close()
{
    GError *error;

    if (input_stream)
        g_input_stream_close(input_stream, NULL, &error);
    if (output_stream)
        g_output_stream_close(output_stream, NULL, &error);
    if (connection)
        g_socket_close(g_socket_connection_get_socket(connection), &error);
}