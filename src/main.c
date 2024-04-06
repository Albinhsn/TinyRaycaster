
#include "arena.h"
#include "common.h"
#include "files.h"
#include "lodepng.h"
#include "string.h"
#include "vector.h"
#include <math.h>

#define COORDINATE_TO_INDEX_2D(x, y, width) (((y) * width) + (x))

static Color colors[7] = {CYAN, GREEN, RED, YELLOW, BLUE, PURPLE, WHITE};

struct Sprite
{
  f64 x, y;
  u64 textureId;
};
typedef struct Sprite Sprite;

struct Map
{
  u8*     tiles;
  Sprite* sprites;
  u64     spriteCount;
  f64     playerX;
  f64     playerY;
  f64     playerA;
  f64     fov;
  u8      width;
  u8      height;
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
  image->width  = 512 * 2;
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

  map->playerX        = 3.456f;
  map->playerY        = 2.345f;
  map->playerA        = 1.523f;
  map->fov            = PI / 3.0f;

  u64        mapSize  = map->height * map->width;
  const char mapstr[] = "0000222222220000"
                        "1              0"
                        "1      11111   0"
                        "1     0        0"
                        "0     0  1110000"
                        "0     3        0"
                        "0   10000      0"
                        "0   3   11100  0"
                        "5   4   0      0"
                        "5   4   1  00000"
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

  map->spriteCount   = 3;
  map->sprites       = ArenaPushArray(arena, Sprite, map->spriteCount);
  Sprite* sprite0    = &map->sprites[0];
  sprite0->x         = 1.834;
  sprite0->y         = 8.765;
  sprite0->textureId = 0;

  Sprite* sprite1    = &map->sprites[1];
  sprite1->x         = 5.323;
  sprite1->y         = 5.365;
  sprite1->textureId = 1;

  Sprite* sprite2    = &map->sprites[2];
  sprite2->x         = 4.123;
  sprite2->y         = 10.265;
  sprite2->textureId = 1;
}

void drawPixelToImageU8(Image* image, u64 x, u64 y, Vec4u8* color)
{
  u64 imageIndex              = COORDINATE_TO_INDEX_2D(x, y, image->width) * 4;
  image->data[imageIndex + 0] = color->r;
  image->data[imageIndex + 1] = color->g;
  image->data[imageIndex + 2] = color->b;
  image->data[imageIndex + 3] = color->a;
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

void drawRectangleToImage(Image* image, u64 x, u64 y, u64 width, u64 height, Color* color)
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

Color sampleTextureFirstPixel(Texture* texture, u64 textureIndex)
{
  Color color           = {};

  u64   widthPerTexture = texture->image.width / texture->textureCount * 4;
  u64   startTextureX   = widthPerTexture * textureIndex;

  u64   offset          = startTextureX;
  u8*   pixel           = &texture->image.data[offset];

  color.r               = pixel[0] / 255.0f;
  color.g               = pixel[1] / 255.0f;
  color.b               = pixel[2] / 255.0f;
  color.a               = pixel[3] / 255.0f;

  return color;
}

inline f64 getDiff(f64 hitX, f64 hitY)
{
  f64 xDiff    = (hitX - (u64)hitX);
  f64 yDiff    = (hitY - (u64)hitY);

  f64 oneXDiff = 1 - xDiff;
  f64 oneYDiff = 1 - yDiff;

  f64 maxX     = MAX(xDiff, oneXDiff);
  f64 maxY     = MAX(yDiff, oneYDiff);

  return maxX > maxY ? yDiff : xDiff;
}

void drawTextureToImage(Texture* texture, Image* image, u64 height, f64 hitX, f64 hitY, u64 textureIndex, u64 pixelX)
{

  f64   diff            = getDiff(hitX, hitY);

  u64   startY          = image->height / 2 - height / 2;
  u64   widthPerTexture = texture->image.width / texture->textureCount;

  u64   xOffset         = (u64)(diff * widthPerTexture) * 4 + textureIndex * widthPerTexture * 4;

  Color color           = {};

  for (u64 yOffset = 0; yOffset < height; yOffset++)
  {
    if (yOffset + startY >= image->height)
    {
      continue;
    }

    f64 sampleY        = ((f64)yOffset / (f64)height);

    u64 yTextureOffset = (texture->image.width * 4) * (u64)(texture->image.height * sampleY);
    u8* pixel          = &texture->image.data[xOffset + yTextureOffset];

    drawPixelToImageU8(image, pixelX, startY + yOffset, (Vec4u8*)pixel);
  }
}

void add3DMapToImage(Map* map, Image* image, Texture* texture)
{
  f64 fovStep           = map->fov / 512.0f;
  i64 screenWidth       = image->width / 2;
  i64 halvedScreenWidth = screenWidth / 2;
  for (i64 i = -halvedScreenWidth; i < halvedScreenWidth; i++)
  {
    f64 r    = map->playerA + i * fovStep;
    f64 step = 0.1f;
    while (step < MAX(map->height, map->width))
    {
      f64 x     = map->playerX + step * cos(r);
      f64 y     = map->playerY + step * sin(r);
      u64 tileX = x;
      u64 tileY = y;

      u8  tile  = map->tiles[COORDINATE_TO_INDEX_2D(tileX, tileY, map->width)];
      if (tile != ' ')
      {

        f64 heightScale = cos(r - map->playerA) * (1 - step / MAX(map->height, map->width)) * 0.25f;
        u64 height      = (u64)((f64)image->height * heightScale);
        u64 pixelX      = i + halvedScreenWidth + screenWidth;
        drawTextureToImage(texture, image, height, x, y, tile - '0', pixelX);
        break;
      }
      step += 0.05f;
    }
  }
}

void add2DMapToImage(Map* map, Image* image, Texture* texture)
{
  u64 height     = map->height;
  u64 width      = map->width;

  u64 tileWidth  = image->width / (width * 2);
  u64 tileHeight = image->height / height;
  for (u64 y = 0; y < height; y++)
  {
    for (u64 x = 0; x < width; x++)
    {
      u64 coordIdx = COORDINATE_TO_INDEX_2D(x, y, width);
      if (map->tiles[coordIdx] != ' ')
      {
        u64   imageX = x * tileWidth;
        u64   imageY = y * tileHeight;
        Color sample = sampleTextureFirstPixel(texture, map->tiles[coordIdx] - '0');
        drawRectangleToImage(image, imageX, imageY, tileWidth, tileHeight, &sample);
      }
    }
  }

  // draw player
  u64 playerX = (map->playerX / map->width) * image->width / 2;
  u64 playerY = (map->playerY / map->height) * image->height;
  drawRectangleToImage(image, playerX - tileWidth / 10, playerY - tileHeight / 10, tileWidth / 5, tileHeight / 5, &GRAY);

  // draw player fov
  f64 fovStep = map->fov / 512.0f;
  for (i64 i = -256; i < 256; i++)
  {
    f64 r    = map->playerA + i * fovStep;
    f64 step = 0.1f;
    while (true)
    {
      f32 x     = map->playerX + step * cos(r);
      f32 y     = map->playerY + step * sin(r);
      u64 tileX = x;
      u64 tileY = y;
      if (map->tiles[COORDINATE_TO_INDEX_2D(tileX, tileY, map->width)] != ' ')
      {
        break;
      }
      u64 imagePlayerFovX = (x / map->width) * image->width / 2;
      u64 imagePlayerFovY = (y / map->height) * image->height;
      drawPixelToImage(image, imagePlayerFovX, imagePlayerFovY, &GRAY);
      step += 0.05f;
    }
  }

  // draw sprites
  u64     spriteCount = map->spriteCount;
  Sprite* sprites     = map->sprites;
  for (u64 i = 0; i < spriteCount; i++)
  {
    u64 spriteX = (sprites[i].x / map->width) * image->width / 2;
    u64 spriteY = (sprites[i].y / map->height) * image->height;
    drawRectangleToImage(image, spriteX - tileWidth / 10, spriteY - tileHeight / 10, tileWidth / 5, tileHeight / 5, &RED);
  }
}

int main()
{
  // ToDo
  // refactor playerX and playerY to be between 0 -> mapWidth and 0 -> mapHeight
  // get the fractional part from hitX
  Image   image        = {};
  Arena   arena        = {};
  Texture texture      = {};
  texture.textureCount = 6;
  int result           = lodepng_decode32_file(&texture.image.data, (u32*)&texture.image.width, (u32*)&texture.image.height, "./walltext.png");
  if (result != 0)
  {
    const char* error = lodepng_error_text(result);
    printf("%s\n", error);
    printf("%d\n", result);
  }

  arena.maxSize = 1024 * 1024 * 4;
  arena.memory  = (u64)malloc(arena.maxSize);
  Map map       = {};

  initMapImage(&arena, &image);
  initMap(&arena, &map, 16, 16);

  add2DMapToImage(&map, &image, &texture);
  add3DMapToImage(&map, &image, &texture);

  String fileName = {};
  sta_initString(&fileName, "out.ppm");
  sta_writePPM(fileName, &image);
}
