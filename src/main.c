
#include "arena.h"
#include "common.h"
#include "files.h"
#include "lodepng.h"
#include "string.h"
#include "vector.h"
#include <math.h>

#define COORDINATE_TO_INDEX_2D(x, y, width) (((y) * width) + (x))

struct Map
{
  u8* tiles;
  f64 playerX;
  f64 playerY;
  f64 playerA;
  f64 fov;
  u8  width;
  u8  height;
};
typedef struct Map Map;

struct Texture
{
  Image image;
  u64   textureCount;
};
typedef struct Texture Texture;

inline u32             packColor(u8 r, u8 g, u8 b, u8 a)
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
      image->data[startIdx + 0] = 0xFF;
      image->data[startIdx + 1] = 0xFF;
      image->data[startIdx + 2] = 0xFF;
      image->data[startIdx + 3] = 0xFF;
    }
  }
}

inline f64 degreesToRadians(f64 degrees)
{

  return degrees * PI / 180.0f;
}

void initMap(Arena* arena, Map* map, u8 width, u8 height)
{
  map->height         = height;
  map->width          = width;

  map->playerX        = 100.0f * (3.456f / 16.0f);
  map->playerY        = 100.0f * (2.345f / 16.0f);
  map->playerA        = 1.523;
  map->fov            = PI / 3;

  u64        mapSize  = map->height * map->width;
  const char mapstr[] = "0000222222220000"
                        "1              0"
                        "1      11111   0"
                        "1     0        0"
                        "0     0  1110000"
                        "0     3        0"
                        "0   10000      0"
                        "0   0   11100  0"
                        "0   0   0      0"
                        "0   0   1  00000"
                        "0       1      0"
                        "2       1      0"
                        "0       0      0"
                        "0 0000000      0"
                        "0              0"
                        "0002222222200000";
  map->tiles          = ArenaPushArray(arena, u8, mapSize);
  for (u64 y = 0; y < height; y++)
  {
    for (u64 x = 0; x < width; x++)
    {
      u64 idx         = COORDINATE_TO_INDEX_2D(x, y, width);
      map->tiles[idx] = mapstr[idx];
    }
  }
}

void drawPixelToImage(Image* image, u64 x, u64 y, Color* color)
{
  u8  r                       = color->r * 255;
  u8  g                       = color->g * 255;
  u8  b                       = color->b * 255;
  u8  a                       = color->a * 255;
  u64 imageIndex              = COORDINATE_TO_INDEX_2D(x, y, image->width) * 4;
  image->data[imageIndex + 0] = r;
  image->data[imageIndex + 1] = g;
  image->data[imageIndex + 2] = b;
  image->data[imageIndex + 3] = a;
}

static Color colors[7] = {CYAN, GREEN, RED, YELLOW, BLUE, WHITE, BLACK};
void         drawRectangleToImage(Image* image, u64 x, u64 y, u64 width, u64 height, Color* color)
{

  for (u64 yOffset = 0; yOffset < height; yOffset++)
  {
    for (u64 xOffset = 0; xOffset < width; xOffset++)
    {
      if (xOffset + x >= image->width || yOffset + y >= image->height)
      {
        continue;
      }
      drawPixelToImage(image, xOffset + x, y + yOffset, color);
    }
  }
}

Color sampleTexture(Texture* texture, u64 textureIndex, f64 x, f64 y)
{
  Color color           = {};

  f64   widthPerTexture = (f64)texture->image.width / (f64)texture->textureCount;
  f64   startTextureX   = widthPerTexture * textureIndex;

  f64   xOffset         = widthPerTexture * x;
  f64   yOffset         = (texture->image.width * texture->image.height * 4) * y;

  u64   offset          = (u64)(yOffset + xOffset);
  printf("Sampling at %lf %lf %lf %lf %ld of %ld\n", x, y, xOffset, yOffset, offset, texture->image.width * texture->image.height * 4);
  u8* pixel = &texture->image.data[offset];

  color.r   = 0xFF * pixel[0];
  color.g   = 0xFF * pixel[1];
  color.b   = 0xFF * pixel[2];
  color.a   = 0xFF * pixel[3];

  return color;
}

void drawTextureToImage(Image* image, u64 x, u64 tileX, u64 tileY, Texture* texture, u64 textureIndex)
{

  // Get the % of the way in we are from tileX -> x
  //  to figure out at what x we want to sample the texture
  u64 y, width, height;
  for (u64 yOffset = 0; yOffset < height; yOffset++)
  {
    for (u64 xOffset = 0; xOffset < width; xOffset++)
    {
      if (xOffset + x >= image->width || yOffset + y >= image->height)
      {
        continue;
      }
      Color color = sampleTexture(texture, textureIndex, xOffset / (f64)width, yOffset / (f64)height);
      drawPixelToImage(image, xOffset + x, y + yOffset, &color);
    }
  }
}

void add3DMapToImage(Map* map, Image* image, Texture* texture, const u64 textureCount)
{
  f64 fovStep = map->fov / 512.0f;
  for (i64 i = -256; i < 256; i++)
  {
    f64 r    = map->playerA + i * fovStep;
    f64 step = 1.0f;
    while (step < 100.0f)
    {
      f64 x     = map->playerX + step * cos(r);
      f64 y     = map->playerY + step * sin(r);
      u64 tileX = (x / 100.0f) * map->width;
      u64 tileY = (y / 100.0f) * map->height;
      u8  tile  = map->tiles[COORDINATE_TO_INDEX_2D(tileX, tileY, map->width)];
      if (tile != ' ')
      {

        drawTextureToImage(image, i + 256, tileX, tileY, texture, tile - '0');
        break;
      }
      step += 0.25f;
    }
  }
}

void add2DMapToImage(Map* map, Image* image)
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
        drawRectangleToImage(image, imageX, imageY, tileWidth, tileHeight, &colors[1]);
      }
    }
  }

  // draw player
  u64 playerX = (map->playerX / 100.0f) * image->width;
  u64 playerY = (map->playerY / 100.0f) * image->height;
  drawRectangleToImage(image, playerX, playerY, tileWidth / 5, tileHeight / 5, 0);

  // draw player fov
  f64 fovStep = map->fov / 512.0f;
  for (i64 i = -256; i < 256; i++)
  {
    f64 r    = map->playerA + i * fovStep;
    f64 step = 1.0f;
    while (true)
    {
      f32 x     = map->playerX + step * cos(r);
      f32 y     = map->playerY + step * sin(r);
      u64 tileX = (x / 100.0f) * map->width;
      u64 tileY = (y / 100.0f) * map->height;
      if (map->tiles[COORDINATE_TO_INDEX_2D(tileX, tileY, width)] != 0)
      {
        break;
      }
      u64 imagePlayerX = (x / 100.0f) * image->width;
      u64 imagePlayerY = (y / 100.0f) * image->height;
      drawPixelToImage(image, imagePlayerX, imagePlayerY, &WHITE);
      step += 0.5f;
    }
  }
}

int main()
{
  Image     image        = {};
  Arena     arena        = {};
  Texture   texture      = {};
  const int textureCount = 6;
  int       result       = lodepng_decode32_file(&texture.image.data, (u32*)&texture.image.width, (u32*)&texture.image.height, "./walltext.png");

  arena.maxSize          = 1024 * 1024 * 4;
  arena.memory           = (u64)malloc(arena.maxSize);
  Map map                = {};

  initMapImage(&arena, &image);
  initMap(&arena, &map, 16, 16);

  add3DMapToImage(&map, &image, &texture, textureCount);

  String fileName = {};
  sta_initString(&fileName, "out.ppm");
  sta_writePPM(fileName, &image);
}
