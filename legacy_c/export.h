#ifndef EXPORT_H
#define EXPORT_H

#include "app.h"

#ifdef __cplusplus
extern "C" {
#endif
int export_results_to_file(const char* path, const DeviceList* list);
int export_results_to_json(const char* path, const DeviceList* list);
#ifdef __cplusplus
}
#endif

#endif // EXPORT_H