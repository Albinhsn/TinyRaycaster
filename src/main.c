
#include "arena.h"
#include "common.h"
#include "files.h"
#include "string.h"
#include "vector.h"

#define COORDINATE_TO_INDEX_2D(x, y, width) (((y) * width) + (x))

struct Map
{
  u8* tiles;
  f64 x;
  f64 y;
  u8  width;
  u8  height;
};
typedef struct Map Map;

inline u32         packColor(u8 r, u8 g, u8 b, u8 a)
{
  return (a << 24) | (b << 16) | (g << 8) | r;
}

inline void unpackColor(Vec4u8* res, u32 color)
{

  res->r = color & 0xFF;
  res->g = (color >> 8) & 0xFF;
  res->b = (color >> 16) & 0xFF;
  res->a = (color >> 24) & 0xFF;
}

void initMapImage(Arena* arena, Image* image)
{
  image->width  = 512;
  image->height = 512;
  u64 imageSize = image->width * image->height * 4;
  image->data   = ArenaPushArray(arena, u8, imageSize);

  for (u64 y = 0; y < image->height; y++)
  {
    for (u64 x = 0; x < image->width; x++)
    {
      u64 startIdx              = (y * image->width + x) * 4;
      image->data[startIdx + 0] = 255 * ((f64)y / (f64)image->height);
      image->data[startIdx + 1] = 255 * ((f64)x / (f64)image->width);
      image->data[startIdx + 2] = 0;
      image->data[startIdx + 3] = 0xFF;
    }
  }
}

void initMap(Arena* arena, Map* map, u8 width, u8 height)
{
  map->height = height;
  map->width  = width;

  map->x      = 20.0f;
  map->y      = 15.0f;

  u64 mapSize = map->height * map->width;
  map->tiles  = ArenaPushArray(arena, u8, mapSize);
  for (u64 i = 0; i < mapSize; i++)
  {
    map->tiles[i] = 0;
  }
  for (u64 i = 0; i < height; i++)
  {
    map->tiles[COORDINATE_TO_INDEX_2D(0, i, width)]          = 1;
    map->tiles[COORDINATE_TO_INDEX_2D(i, 0, width)]          = 1;
    map->tiles[COORDINATE_TO_INDEX_2D(width - 1, i, width)]  = 1;
    map->tiles[COORDINATE_TO_INDEX_2D(i, height - 1, width)] = 1;
  }

  for (u64 i = 0; i < 6; i++)
  {
    map->tiles[COORDINATE_TO_INDEX_2D(i + 2, height - 3, width)] = 1;
  }

  for (u64 i = 0; i < 8; i++)
  {
    map->tiles[COORDINATE_TO_INDEX_2D(8, height - 3 - i, width)] = 1;
    map->tiles[COORDINATE_TO_INDEX_2D(9 + i, 4, width)]          = 1;
  }

  for (u64 i = 0; i < 5; i++)
  {
    map->tiles[COORDINATE_TO_INDEX_2D(8 + i, height - 3 - 6, width)]  = 1;
    map->tiles[COORDINATE_TO_INDEX_2D(8 - i, height - 3 - 7, width)]  = 1;
    map->tiles[COORDINATE_TO_INDEX_2D(11 + i, height - 3 - 4, width)] = 1;
    map->tiles[COORDINATE_TO_INDEX_2D(7 + i, 2, width)]               = 1;
  }
  for (u64 i = 0; i < 4; i++)
  {
    map->tiles[COORDINATE_TO_INDEX_2D(4, height - 10 + i, width)] = 1;
    map->tiles[COORDINATE_TO_INDEX_2D(6, height - 10 - i, width)] = 1;
  }
}

void drawRectangleToImage(Image* image, u64 imageX, u64 imageY, u64 width, u64 height, Color* color)
{
  u8 r = color->r * 255;
  u8 g = color->g * 255;
  u8 b = color->b * 255;
  u8 a = color->a * 255;
  for (u64 y = 0; y < height; y++)
  {
    for (u64 x = 0; x < width; x++)
    {
      u64 imageIndex              = COORDINATE_TO_INDEX_2D(x + imageX, y + imageY, image->width) * 4;
      image->data[imageIndex + 0] = r;
      image->data[imageIndex + 1] = g;
      image->data[imageIndex + 2] = b;
      image->data[imageIndex + 3] = a;
    }
  }
}

void addTilesToMapImage(Map* map, Image* image)
{
  u64 height     = map->height;
  u64 width      = map->width;

  u64 tileWidth  = image->width / width;
  u64 tileHeight = image->height / height;
  for (u64 y = 0; y < height; y++)
  {
    for (u64 x = 0; x < width; x++)
    {
      if (map->tiles[COORDINATE_TO_INDEX_2D(x, y, width)] != 0)
      {
        u64 imageX = x * tileWidth;
        u64 imageY = y * tileHeight;
        drawRectangleToImage(image, imageX, imageY, tileWidth, tileHeight, &CYAN);
      }
    }
  }

  // draw player
  u64 playerX = (map->x / 100.0f) * image->width;
  u64 playerY = (map->y / 100.0f) * image->height;
  drawRectangleToImage(image, playerX, playerY, tileWidth / 5, tileHeight / 5, &WHITE);
}

int main()
{
  Image image   = {};
  Arena arena   = {};
  arena.maxSize = 1024 * 1024 * 4;
  arena.memory  = (u64)malloc(arena.maxSize);
  Map map       = {};

  initMapImage(&arena, &image);
  initMap(&arena, &map, 16, 16);

  addTilesToMapImage(&map, &image);

  String fileName = {};
  sta_initString(&fileName, "out.ppm");
  sta_writePPM(fileName, &image);
}
