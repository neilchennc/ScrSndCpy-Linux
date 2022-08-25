#ifndef TRACK_DEVICE_H
#define TRACK_DEVICE_H

#include <gtk/gtk.h>

#define MAX_DEVICE_NAME_LENGTH 32
#define MAX_NUMBER_OF_DEVICES 32

typedef struct
{
    char name[MAX_NUMBER_OF_DEVICES][MAX_DEVICE_NAME_LENGTH]; // device name
    int device_size;                                          // Number of devices
} adb_device_t;

void track_device_init(GSourceFunc new_device, GSourceFunc disconnected);
void track_device_start();
void track_device_close();

#endif // TRACK_DEVICE_H