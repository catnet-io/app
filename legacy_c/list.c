#include "app.h"
#include <stdlib.h>

void device_list_init(DeviceList* list) {
    list->items = NULL; list->count = 0; list->capacity = 0;
}

void device_list_clear(DeviceList* list) {
    if (list->items) { free(list->items); list->items = NULL; }
    list->count = 0; list->capacity = 0;
}

void device_list_push(DeviceList* list, const DeviceInfo* info) {
    if (list->count + 1 > list->capacity) {
        size_t newcap = list->capacity ? list->capacity * 2 : 64;
        DeviceInfo* newitems = (DeviceInfo*)realloc(list->items, newcap * sizeof(DeviceInfo));
        if (!newitems) return;
        list->items = newitems; list->capacity = newcap;
    }
    list->items[list->count++] = *info;
}