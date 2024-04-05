#include "files.h"
#include "arena.h"
#include <string.h>
bool sta_writePPM(String fileName, Image *image) {
  char buffer[256] = {};
  strncpy(&buffer[0], &fileName.buffer[0], fileName.len);

  FILE *filePtr = fopen(buffer, "w");
  if (filePtr == 0) {
    printf("Failed to open %.*s\n", (i32)fileName.len, fileName.buffer);
    return false;
  }

  int result =
      fprintf(filePtr, "P3\n%ld %ld\n255\n", image->width, image->height);
  if (result < 0) {
    return false;
  }

  u64 imageSize = image->height * image->width * 4;
  u8 *imageData = image->data;

  for (u64 i = 0; i < imageSize; i += 4) {
    result = fprintf(filePtr, "%d %d %d\n", imageData[i + 0],
                     imageData[i + 1], imageData[i + 2]);
    if (result < 0) {
      return false;
    }
  }

  (void)fclose(filePtr);

  return true;
}

bool sta_readFile(Arena *arena, struct String *string, const char *fileName) {
  FILE *filePtr;
  long fileSize, count;
  char *buffer;
  int error;

  filePtr = fopen(fileName, "r");
  if (!filePtr) {
    return NULL;
  }

  fileSize = fseek(filePtr, 0, SEEK_END);
  fileSize = ftell(filePtr);

  string->len = fileSize;
  string->buffer = ArenaPushArray(arena, char, fileSize + 1);
  string->buffer[fileSize] = '\0';
  fseek(filePtr, 0, SEEK_SET);
  count = fread(string->buffer, 1, fileSize, filePtr);
  if (count != fileSize) {
    free(string->buffer);
    return false;
  }

  error = fclose(filePtr);
  if (error != 0) {
    ArenaPop(arena, fileSize + 1);
    return false;
  }

  return true;
}

bool sta_loadTarga(Arena *arena, Image *image, const char *filename) {

  struct TargaHeader targaFileHeader;

  FILE *filePtr;
  unsigned long count, imageSize;

  filePtr = fopen(filename, "rb");
  if (filePtr == NULL) {
    printf("ERROR: file doesn't exist %s\n", filename);
    return false;
  }

  count = fread(&targaFileHeader, sizeof(struct TargaHeader), 1, filePtr);
  if (count != 1) {
    printf("ERROR: Failed to read into header\n");
    return false;
  }

  image->width = targaFileHeader.width;
  image->height = targaFileHeader.height;
  image->bpp = targaFileHeader.imagePixelSize;

  if (image->bpp == 32) {
    imageSize = image->width * image->height * 4;
  } else if (image->bpp == 24) {
    imageSize = image->width * image->height * 3;
  }

  image->data = ArenaPushArray(arena, unsigned char, imageSize);

  count = fread(image->data, 1, imageSize, filePtr);
  if (count != imageSize) {
    printf("ERROR: count read doesn't equal imageSize\n");
    return false;
  }

  if (fclose(filePtr) != 0) {
    return false;
  }

  for (int idx = 0; idx < image->height * image->width; idx++) {
    unsigned char tmp = image->data[idx];
    image->data[idx] = image->data[idx + 2];
    image->data[idx + 2] = tmp;
  }

  return image;
}
