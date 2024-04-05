#ifndef STA_STRING_H
#define STA_STRING_H

#include "arena.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
typedef uint64_t u64;
typedef int      i32;

struct String
{
  u64   len;
  u64   capacity;
  char* buffer;
};
typedef struct String String;

struct StringArray
{
  String* str;
  u64     len;
  u64     capacity;
};
typedef struct StringArray StringArray;

void                       sta_initString(String* s, const char* msg);
bool                       sta_strncmp(String* s1, String* s2, u64 len);
void                       sta_strncpy(Arena* arena, String* res, String* source, i32 n);
void                       sta_strcpy(Arena* arena, String* res, String* source);
void                       sta_strsplit(Arena* arena, StringArray* res, String* s1, char c);
void                       sta_strrchr(Arena* arena, String* res, String* s1, char c);
void                       sta_strstr(Arena* arena, String* res, String* haystack, String* needle);
void                       sta_strchr(Arena* arena, String* res, String* s1, char c);
void                       sta_strncat(Arena* arena, String* s1, String* s2, u64 n);
void                       sta_strcat(Arena* arena, String* s1, String* s2);
i32                        sta_strcmpi32(String* s1, String* s2);
bool                       sta_strcmp(String* s1, String* s2);
i32                        sta_strncmpi32(String* s1, String* s2, u64 len);

#endif
