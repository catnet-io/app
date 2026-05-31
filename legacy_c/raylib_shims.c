#include "raylib_shims.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

// --- Raylib compatibility shims ---
// Implementacoes minimas para satisfazer os simbolos esperados pelos modulos
// do raylib quando compilando sem raylib.lib precompilada.

void TraceLog(int logType, const char* text, ...)
{
    (void)logType;
    if (!text) return;
    va_list args;
    va_start(args, text);
    vprintf(text, args);
    printf("\n");
    fflush(stdout);
    va_end(args);
}

void* MemAlloc(unsigned int size)
{
    return calloc(size, 1);
}

void* MemRealloc(void* ptr, unsigned int size)
{
    return realloc(ptr, size);
}

void MemFree(void* ptr)
{
    free(ptr);
}

unsigned char* LoadFileData(const char* fileName, int* dataSize)
{
    if (dataSize) *dataSize = 0;
    if (!fileName) return NULL;
    FILE* f = fopen(fileName, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    rewind(f);
    unsigned char* data = (unsigned char*)malloc((size_t)sz);
    if (!data) { fclose(f); return NULL; }
    size_t rd = fread(data, 1, (size_t)sz, f);
    fclose(f);
    if (dataSize) *dataSize = (int)rd;
    return data;
}

void UnloadFileData(unsigned char* data)
{
    free(data);
}

bool SaveFileData(const char* fileName, void* data, int dataSize)
{
    if (!fileName || !data || dataSize < 0) return false;
    FILE* f = fopen(fileName, "wb");
    if (!f) return false;
    size_t wr = fwrite(data, 1, (size_t)dataSize, f);
    fclose(f);
    return (wr == (size_t)dataSize);
}

char* LoadFileText(const char* fileName)
{
    if (!fileName) return NULL;
    FILE* f = fopen(fileName, "rt");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    rewind(f);
    char* text = (char*)malloc((size_t)sz + 1);
    if (!text) { fclose(f); return NULL; }
    size_t rd = fread(text, 1, (size_t)sz, f);
    text[rd] = '\0';
    fclose(f);
    return text;
}

void UnloadFileText(char* text)
{
    free(text);
}

bool SaveFileText(const char* fileName, const char* text)
{
    if (!fileName || !text) return false;
    FILE* f = fopen(fileName, "wt");
    if (!f) return false;
    int r = fputs(text, f);
    fclose(f);
    return (r >= 0);
}
