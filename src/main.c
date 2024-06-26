
#include "arena.h"
#include "common.h"
#include "files.h"
#include "lodepng.h"
#include "string.h"
#include "vector.h"
#include <math.h>

struct Sprite
{
  f64 x, y;
  u64 textureId;
};
typedef struct Sprite Sprite;

inline f64            getDistance(f64 x0, f64 y0, f64 x1, f64 y1)
{

  f64 xDiff = x0 - x1;
  f64 yDiff = y0 - y1;
  return sqrt(xDiff * xDiff + yDiff * yDiff);
}

f64 playerX;
f64 playerY;

int cmpSprite(const void* a_, const void* b_)
{
  Sprite* a = (Sprite*)a_;
  Sprite* b = (Sprite*)b_;

  return getDistance(playerX, playerY, b->x, b->y) - getDistance(playerX, playerY, a->x, a->y);
}

#define COORDINATE_TO_INDEX_2D(x, y, width) (((y) * width) + (x))

static Color colors[7] = {CYAN, GREEN, RED, YELLOW, BLUE, PURPLE, WHITE};

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

struct Framebuffer
{
  u64            width, height;
  unsigned char* data;
  f64*           zBuffer;
};
typedef struct Framebuffer Framebuffer;

inline u32                 packColor(u8 r, u8 g, u8 b, u8 a)
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

void initMapImage(Arena* arena, Framebuffer* image)
{
  image->width   = 512 * 2;
  image->height  = 512;
  u64 imageSize  = image->width * image->height * 4;
  image->data    = ArenaPushArray(arena, u8, imageSize);
  image->zBuffer = ArenaPushArray(arena, f64, image->width / 2);

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

  map->spriteCount   = 4;
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

  Sprite* sprite3    = &map->sprites[3];
  sprite3->x         = 3.023;
  sprite3->y         = 3.812;
  sprite3->textureId = 2;
}

void drawPixelToFramebufferU8(Framebuffer* image, u64 x, u64 y, Vec4u8* color)
{
  u64 imageIndex              = COORDINATE_TO_INDEX_2D(x, y, image->width) * 4;
  image->data[imageIndex + 0] = color->r;
  image->data[imageIndex + 1] = color->g;
  image->data[imageIndex + 2] = color->b;
  image->data[imageIndex + 3] = color->a;
}
void drawPixelToFramebuffer(Framebuffer* framebuffer, u64 x, u64 y, Color* color)
{
  u8  r                             = color->r * 255;
  u8  g                             = color->g * 255;
  u8  b                             = color->b * 255;
  u8  a                             = color->a * 255;
  u64 imageIndex                    = COORDINATE_TO_INDEX_2D(x, y, framebuffer->width) * 4;
  framebuffer->data[imageIndex + 0] = r;
  framebuffer->data[imageIndex + 1] = g;
  framebuffer->data[imageIndex + 2] = b;
  framebuffer->data[imageIndex + 3] = a;
}

void drawRectangleToImage(Framebuffer* framebuffer, u64 x, u64 y, u64 width, u64 height, Color* color)
{

  for (u64 yOffset = 0; yOffset < height; yOffset++)
  {
    for (u64 xOffset = 0; xOffset < width; xOffset++)
    {
      if (xOffset + x >= framebuffer->width || yOffset + y >= framebuffer->height)
      {
        continue;
      }
      drawPixelToFramebuffer(framebuffer, xOffset + x, y + yOffset, color);
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

void drawTextureToImage(Texture* texture, Framebuffer* framebuffer, u64 height, f64 hitX, f64 hitY, u64 textureIndex, u64 pixelX)
{

  f64 diff            = getDiff(hitX, hitY);

  u64 startY          = framebuffer->height / 2 - height / 2;
  u64 widthPerTexture = texture->image.width / texture->textureCount;

  u64 xOffset         = (u64)(diff * widthPerTexture) * 4 + textureIndex * widthPerTexture * 4;

  for (u64 yOffset = 0; yOffset < height; yOffset++)
  {
    if (yOffset + startY >= framebuffer->height)
    {
      continue;
    }

    f64 sampleY        = ((f64)yOffset / (f64)height);

    u64 yTextureOffset = (texture->image.width * 4) * (u64)(texture->image.height * sampleY);
    u8* pixel          = &texture->image.data[xOffset + yTextureOffset];

    drawPixelToFramebufferU8(framebuffer, pixelX, startY + yOffset, (Vec4u8*)pixel);
  }
}

void drawSprite(Map* map, Framebuffer* framebuffer, Sprite* sprite, Texture* monsters)
{
  f64 xDiff     = sprite->x - map->playerX;
  f64 yDiff     = sprite->y - map->playerY;

  f64 spriteDir = atan2(yDiff, xDiff);
  while (spriteDir - map->playerA > PI)
  {
    spriteDir -= PI * 0.5f;
  }
  while (spriteDir - map->playerA < -PI)
  {
    spriteDir += PI * 0.5f;
  }

  f64 spriteDist      = sqrt(xDiff * xDiff + yDiff * yDiff);
  f64 spriteSize      = MIN(1000, (u64)(framebuffer->height / spriteDist));
  f64 dirDiff         = spriteDir - map->playerA;

  u64 centerX         = framebuffer->width - (f64)framebuffer->width / 4 - spriteSize / 2;
  u64 x               = centerX + (dirDiff / map->fov) * ((f64)framebuffer->width / 2);
  u64 centerY         = (framebuffer->height - spriteSize) / 2;
  u64 width           = spriteSize;
  u64 height          = spriteSize;

  u64 widthPerTexture = monsters->image.width / monsters->textureCount;

  for (u64 yOffset = 0; yOffset < height; yOffset++)
  {
    for (u64 xOffset = 0; xOffset < width; xOffset++)
    {
      if (xOffset + x >= framebuffer->width || yOffset + centerY >= framebuffer->height)
      {
        continue;
      }
      if (xOffset + x - framebuffer->width / 2 >= 0 && framebuffer->zBuffer[xOffset + x - framebuffer->width / 2] >= spriteDist)
      {

        f64 sampleY        = ((f64)yOffset / (f64)height);
        f64 sampleX        = ((f64)xOffset / (f64)width);

        u64 yTextureOffset = (monsters->image.width * 4) * (u64)(monsters->image.height * sampleY);
        u64 xTextureOffset = (sprite->textureId * widthPerTexture * 4) + 4 * (u64)(widthPerTexture * sampleX);

        u8* sample         = &monsters->image.data[xTextureOffset + yTextureOffset];

        if (sample[3] != 0)
        {
          drawPixelToFramebufferU8(framebuffer, xOffset + x, centerY + yOffset, (Vec4u8*)sample);
        }
      }
    }
  }
}

void add3DMapToImage(Map* map, Framebuffer* framebuffer, Texture* textureWall, Texture* textureMonster)
{
  f64 fovStep           = map->fov / 512.0f;
  i64 screenWidth       = framebuffer->width / 2;
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
        framebuffer->zBuffer[i + 256] = getDistance(x, y, map->playerX, map->playerY);
        f64 heightScale               = cos(r - map->playerA) * (1 - step / MAX(map->height, map->width)) * 0.25f;
        u64 height                    = (u64)((f64)framebuffer->height * heightScale);
        u64 pixelX                    = i + halvedScreenWidth + screenWidth;
        drawTextureToImage(textureWall, framebuffer, height, x, y, tile - '0', pixelX);
        break;
      }
      step += 0.05f;
    }
  }

  u64 spriteCount = map->spriteCount;
  playerX         = map->playerX;
  playerY         = map->playerY;
  qsort(map->sprites, spriteCount, sizeof(Sprite), cmpSprite);
  Sprite* sprites = map->sprites;
  for (u64 i = 0; i < spriteCount; i++)
  {
    drawSprite(map, framebuffer, &sprites[i], textureMonster);
  }
}

void add2DMapToImage(Map* map, Framebuffer* image, Texture* texture)
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
      drawPixelToFramebuffer(image, imagePlayerFovX, imagePlayerFovY, &GRAY);
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
  Framebuffer framebuffer  = {};
  Arena       arena        = {};
  Texture     textureWall  = {};
  textureWall.textureCount = 6;
  int result               = lodepng_decode32_file(&textureWall.image.data, (u32*)&textureWall.image.width, (u32*)&textureWall.image.height, "./walltext.png");
  if (result != 0)
  {
    const char* error = lodepng_error_text(result);
    printf("%s\n", error);
    printf("%d\n", result);
    return 1;
  }

  Texture textureMonster      = {};
  textureMonster.textureCount = 4;
  result                      = lodepng_decode32_file(&textureMonster.image.data, (u32*)&textureMonster.image.width, (u32*)&textureMonster.image.height, "./monsters.png");
  if (result != 0)
  {
    const char* error = lodepng_error_text(result);
    printf("%s\n", error);
    printf("%d\n", result);
    return 1;
  }

  arena.maxSize = 1024 * 1024 * 4;
  arena.memory  = (u64)malloc(arena.maxSize);
  Map map       = {};

  initMapImage(&arena, &framebuffer);
  initMap(&arena, &map, 16, 16);

  add2DMapToImage(&map, &framebuffer, &textureWall);
  add3DMapToImage(&map, &framebuffer, &textureWall, &textureMonster);

  String fileName = {};
  sta_initString(&fileName, "out.ppm");

  Image image  = {};
  image.width  = framebuffer.width;
  image.height = framebuffer.height;
  image.data   = framebuffer.data;
  image.bpp    = 32;

  sta_writePPM(fileName, &image);
}
