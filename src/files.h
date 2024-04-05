#ifndef STA_FILES_H
#define STA_FILES_H
#include "common.h"
#include "string.h"
#include <stdbool.h>
#include <stdio.h>

struct Image {
  u64 width, height;
  i32 bpp;
  unsigned char *data;
};
typedef struct Image Image;
struct TargaHeader {
  union {
    u8 header[18];
    struct {
      u8 charactersInIdentificationField;
      u8 colorMapType;
      u8 imageType;
      u8 colorMapSpec[5];
      u16 xOrigin;
      u16 yOrigin;
      u16 width;
      u16 height;
      u8 imagePixelSize;
      u8 imageDescriptor;
    };
  };
};
typedef struct TargaHeader TargaHeader;

bool sta_loadTarga(Arena *arena, struct Image *image, const char *filename);
bool sta_readFile(Arena *arena, struct String *string, const char *fileName);
bool sta_appenddToFile(String fileName, String message);
bool sta_writePPM(String fileName, Image *image);

#endif
