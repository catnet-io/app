#ifndef RAYLIB_SHIMS_H
#define RAYLIB_SHIMS_H

// Shims de compatibilidade para simbolos esperados pelos modulos do raylib.
// Estas implementacoes substituem os simbolos do raylib quando compilando
// os modulos do raylib junto com o projeto sem uma raylib.lib precompilada.
// NAO inclua raylib.h aqui — este header declara os proprios shims.

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void  TraceLog(int logType, const char* text, ...);
void* MemAlloc(unsigned int size);
void* MemRealloc(void* ptr, unsigned int size);
void  MemFree(void* ptr);

unsigned char* LoadFileData(const char* fileName, int* dataSize);
void           UnloadFileData(unsigned char* data);
bool           SaveFileData(const char* fileName, void* data, int dataSize);

char* LoadFileText(const char* fileName);
void  UnloadFileText(char* text);
bool  SaveFileText(const char* fileName, const char* text);

#ifdef __cplusplus
}
#endif

#endif // RAYLIB_SHIMS_H
