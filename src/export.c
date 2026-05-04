#include "export.h"
#include <stdio.h>

int export_results_to_file(const char* path, const DeviceList* list) {
    FILE* f = fopen(path, "w");
    if (!f) return 0;
    fprintf(f, "IP;Hostname;MAC;Status;Ports\n");
    for (size_t i=0; i<list->count; ++i) {
        const DeviceInfo* di = &list->items[i];
        fprintf(f, "%s;%s;%s;%s;", di->ip, di->hostname[0]?di->hostname:"", di->mac[0]?di->mac:"", di->is_alive?"UP":"DOWN");
        for (int p=0; p<di->open_ports_count; ++p) {
            fprintf(f, "%d%s", di->open_ports[p], (p<di->open_ports_count-1?",":""));
        }
        fprintf(f, "\n");
    }
    fclose(f);
    return 1;
}

static void json_write_string(FILE* f, const char* s)
{
    fputc('"', f);
    if (s)
    {
        for (; *s; ++s)
        {
            unsigned char c = (unsigned char)*s;
            if (c == '"')
                fputs("\\\"", f);
            else if (c == '\\')
                fputs("\\\\", f);
            else if (c == '\n')
                fputs("\\n", f);
            else if (c == '\r')
                fputs("\\r", f);
            else if (c == '\t')
                fputs("\\t", f);
            else if (c < 0x20)
                fprintf(f, "\\u%04x", (unsigned int)c);
            else
                fputc(c, f);
        }
    }
    fputc('"', f);
}

int export_results_to_json(const char* path, const DeviceList* list)
{
    FILE* f = fopen(path, "w");
    if (!f) return 0;

    fputs("{\"devices\":[\n", f);

    for (size_t i = 0; i < list->count; ++i)
    {
        const DeviceInfo* di = &list->items[i];

        fputs("  {", f);

        fputs("\"ip\":", f);
        json_write_string(f, di->ip);

        fputs(",\"hostname\":", f);
        json_write_string(f, di->hostname[0] ? di->hostname : "");

        fputs(",\"mac\":", f);
        json_write_string(f, di->mac[0] ? di->mac : "");

        fputs(",\"ports\":[", f);
        for (int p = 0; p < di->open_ports_count; ++p)
        {
            if (p > 0) fputc(',', f);
            fprintf(f, "%d", di->open_ports[p]);
        }
        fputs("]", f);

        fputs("}", f);
        if (i < list->count - 1)
            fputs(",\n", f);
        else
            fputc('\n', f);
    }

    fputs("]}", f);

    fclose(f);
    return 1;
}