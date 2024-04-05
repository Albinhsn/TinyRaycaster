#include "string.h"
#include "arena.h"

void sta_initString(String *s, const char *msg) {
  s->buffer = (char *)msg;
  s->len = strlen(msg);
  s->capacity = s->len;
}
void sta_strInit(Arena *arena, String *s1, const char *str) {
  u64 len = strlen(str);
  String tmp = (String){.len = len, .buffer = (char *)str, .capacity = len};
  sta_strcpy(arena, s1, &tmp);
}

bool sta_strncmp(String *s1, String *s2, u64 len) {
  u64 minLen = s1->len < s2->len ? s1->len : s2->len;
  return minLen >= len && strncmp(s1->buffer, s2->buffer, len) == 0;
}

void sta_strncpy(Arena *arena, String *res, String *source, i32 n) {
  res->len = n;
  res->capacity = n;
  res->buffer = ArenaPushArray(arena, char, n);
  memset(res->buffer, 0, n);
  memcpy(res->buffer, source->buffer, n);
}

void sta_strcpy(Arena *arena, String *res, String *source) {
  res->len = source->len;
  res->capacity = source->capacity;
  res->buffer = ArenaPushArray(arena, char, source->capacity);
  memcpy(res->buffer, source->buffer, source->len);
}

void sta_strsplit(Arena *arena, StringArray *res, String *s1, char c) {
  res->str = ArenaPushStruct(arena, String);
  res->len = 0;
  res->capacity = 1;

  u64 prevIdx = 0;
  for (u64 idx = 0; idx < s1->len; idx++) {
    if (s1->buffer[idx] == c) {
      if (prevIdx == idx) {
        prevIdx = idx;
        continue;
      }
      ArenaPushStruct(arena, String);
      res->str[res->len] = (String){.len = idx - prevIdx,
                                    .buffer = &s1->buffer[prevIdx],
                                    .capacity = idx - prevIdx};
      res->len++;
      res->capacity++;

      prevIdx = idx + 1;
    }
  }
  String tmp = (String){
      .len = s1->len - prevIdx, .buffer = &s1->buffer[prevIdx], .capacity = 0};
  sta_strncpy(arena, &res->str[res->len], &tmp, s1->len - prevIdx);
  res->len++;
  res->capacity++;
}

void sta_strrchr(Arena *arena, String *res, String *s1, char c) {
  for (u64 idx = s1->len - 1; idx >= 0; idx--) {
    if (s1->buffer[idx] == c) {
      res->len = s1->len - idx;
      res->buffer = ArenaPushArray(arena, char, res->len);
      res->capacity = res->len;
      return;
    }
  }
  res->buffer = 0;
}

void sta_strstr(Arena *arena, String *res, String *haystack, String *needle) {
  String tmp = {};
  tmp.len = haystack->len;
  tmp.buffer = haystack->buffer;
  for (u64 idx = 0; idx < haystack->len - needle->len; idx++) {
    if (sta_strncmp(&tmp, needle, needle->len)) {
      sta_strcpy(arena, res, &tmp);
      return;
    }
    tmp.len--;
    tmp.buffer++;
  }
  res->buffer = 0;
}

void sta_strchr(Arena *arena, String *res, String *s1, char c) {
  for (u64 idx = 0; idx < s1->len; idx++) {
    if (s1->buffer[idx] == c) {
      res->len = idx + 1;
      res->buffer = ArenaPushArray(arena, char, res->len);
      res->capacity = res->len;
      return;
    }
  }
  res->buffer = 0;
}
void sta_strncat(Arena *arena, String *s1, String *s2, u64 n) {
  u64 prevLen = s1->len;
  s1->len += n;
  if (s1->len >= s1->capacity) {
    s1->capacity = s1->len;
    char *prevBuffer = s1->buffer;
    s1->buffer = ArenaPushArray(arena, char, s1->len);
    memcpy(s1->buffer, prevBuffer, prevLen);
  }
  memcpy(&s1->buffer[prevLen], s2->buffer, n);
}

void sta_strcat(Arena *arena, String *s1, String *s2) {
  u64 prevLen = s1->len;
  s1->len += s2->len;
  if (s1->len >= s1->capacity) {
    s1->capacity = s1->len;
    char *prevBuffer = s1->buffer;
    s1->buffer = ArenaPushArray(arena, char, s1->len);
    memcpy(s1->buffer, prevBuffer, prevLen);
  }
  memcpy(&s1->buffer[prevLen], s2->buffer, s2->len);
}

i32 sta_strcmpi32(String *s1, String *s2) {
  u64 minLen = s1->len < s2->len ? s1->len : s2->len;
  i32 cmp = strncmp(s1->buffer, s2->buffer, minLen);
  return cmp != 0 ? cmp : (s1->len > s2->len ? 1 : -1);
}

bool sta_strcmp(String *s1, String *s2) {
  u64 minLen = s1->len < s2->len ? s1->len : s2->len;
  return s1->len == s2->len && strncmp(s1->buffer, s2->buffer, minLen) == 0;
}

i32 sta_strncmpi32(String *s1, String *s2, u64 len) {
  return strncmp(s1->buffer, s2->buffer, len);
}
