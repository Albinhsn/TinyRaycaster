#ifndef VECTOR_H
#define VECTOR_H

#include "common.h"

#define CREATE_VEC2i32(x, y)       ((struct Vec2i32){x, y})
#define CREATE_VEC3i32(x, y, z)    ((struct Vec3i32){x, y, z})
#define CREATE_VEC4i32(x, y, z, w) ((struct Vec4i32){x, y, z, w})

#define CREATE_VEC2f32(x, y)       ((struct Vec2f32){x, y})
#define CREATE_VEC3f32(x, y, z)    ((struct Vec3f32){x, y, z})
#define CREATE_VEC4f32(x, y, z, w) ((struct Vec4f32){x, y, z, w})

typedef struct Vec4f32;

#define RED    ((struct Vec4f32){1.0f, 0.0f, 0.0f, 1.0f})
#define YELLOW ((struct Vec4f32){1.0f, 1.0f, 0.0f, 1.0f})
#define GREEN  ((struct Vec4f32){0.0f, 1.0f, 0.0f, 1.0f})
#define CYAN   ((struct Vec4f32){0.0f, 1.0f, 1.0f, 1.0f})
#define PURPLE ((struct Vec4f32){1.0f, 0.0f, 1.0f, 1.0f})
#define BLUE   ((struct Vec4f32){1.0f, 0.0f, 0.0f, 1.0f})
#define BLACK ((struct Vec4f32){0.0f, 0.0f, 0.0f, 0.0f})
#define WHITE ((struct Vec4f32){1.0f, 1.0f, 1.0f, 1.0f})
#define GRAY ((struct Vec4f32){0.5f, 0.5f, 0.5f, 0.5f})

struct Vec4u8
{
  union
  {
    u8 pos[4];
    struct
    {
      u8 x;
      u8 y;
      u8 z;
      u8 w;
    };
    struct
    {
      u8 r;
      u8 g;
      u8 b;
      u8 a;
    };
  };
};
typedef struct Vec4u8 Vec4u8;

struct Vec4f32
{
  union
  {
    f32 pos[4];
    struct
    {
      f32 x;
      f32 y;
      f32 z;
      f32 w;
    };
    struct
    {
      f32 r;
      f32 g;
      f32 b;
      f32 a;
    };
  };
};
typedef struct Vec4f32 Vec4f32;

struct Vec3u32
{
  union
  {
    u32 pos[3];
    struct
    {
      u32 x;
      u32 y;
      u32 z;
    };
    struct
    {
      u32 r;
      u32 g;
      u32 b;
    };
  };
};
typedef struct Vec3u32 Vec3u32;

struct Vec3f32
{
  union
  {
    f32 pos[3];
    struct
    {
      f32 x;
      f32 y;
      f32 z;
    };
    struct
    {
      f32 r;
      f32 g;
      f32 b;
    };
  };
};
typedef struct Vec3f32 Vec3f32;
typedef struct Vec4f32 Color;

struct Vec2f32
{
  union
  {
    f32 pos[2];
    struct
    {
      f32 x;
      f32 y;
    };
  };
};
typedef struct Vec2f32 Vec2f32;

struct Vec2i32
{
  union
  {
    i32 pos[2];
    struct
    {
      i32 x;
      i32 y;
    };
  };
};
typedef struct Vec2i32 Vec2i32;

struct Vec3i32
{
  i32 x, y, z;
};
typedef struct Vec3i32 Vec3i32;

struct Vec4i32
{
  i32 x, y, z, w;
};
typedef struct Vec4i32 Vec4i32;

struct Vec2i8
{
  union
  {
    i8 pos[2];
    struct
    {
      i8 x;
      i8 y;
    };
  };
};
typedef struct Vec2i8 Vec2i8;

struct Vec3u8
{
  union
  {
    u8 pos[3];
    struct
    {
      u8 x;
      u8 y;
      u8 z;
    };
  };
};
typedef struct Vec3u8 Vec3u8;

struct Vec2u8
{
  union
  {
    u8 pos[2];
    struct
    {
      u8 x;
      u8 y;
    };
  };
};
typedef struct Vec2u8 Vec2u8;

struct Matrix4x4
{
  union
  {
    f32 m[4][4];
    struct
    {
      f32 i[4];
      f32 j[4];
      f32 k[4];
      f32 l[4];
    };
  };
};
typedef struct Matrix4x4 Matrix4x4;

struct Matrix3x3
{
  union
  {
    f32 a[9];
    f32 m[3][3];
    struct
    {
      f32 i[3];
      f32 j[3];
      f32 k[3];
    };
  };
};
typedef struct Matrix3x3 Matrix3x3;

void                     debugVec2f32(struct Vec2f32 v);
void                     debugVec3f32(struct Vec3f32 v);
void                     debugVec3i32(struct Vec3i32 v);
void                     debugVec4f32(struct Vec4f32 v);
void                     debugMatrix4x4(struct Matrix4x4 res);
void                     clearMat3x3(Matrix3x3* m);
void                     debugMatrix3x3(struct Matrix3x3 m);

void                     buildIdentityMatrix4x4(struct Matrix4x4* m);
void                     buildViewportMatrix4x4(struct Matrix4x4* m, i32 x, i32 y, i32 w, i32 h, i32 depth);

struct Matrix4x4         lookAt(struct Vec3f32 eye, struct Vec3f32 center, struct Vec3f32 up);

struct Vec3f32           VecMul3f32(struct Vec3f32 a, struct Vec3f32 b);

struct Vec3f32           ProjectVec4ToVec3(struct Vec4f32 v);
struct Vec2f32           ProjectVec3ToVec2(struct Vec3f32 v);

struct Matrix4x4         invertMat4x4(struct Matrix4x4 m);
struct Matrix4x4         transposeMat4x4(struct Matrix4x4 m);
struct Matrix3x3         transposeMat3x3(struct Matrix3x3 m);
struct Matrix3x3         invertMat3x3(struct Matrix3x3 m);
void                     MatMul3x3(struct Matrix3x3* res, struct Matrix3x3* m1, struct Matrix3x3* m2);
void                     MatMul3x32(Matrix3x3* res, Matrix3x3* a, Matrix3x3* b);
void                     MatMul3x33(Matrix3x3* res, Matrix3x3* a, Matrix3x3* b);
void                     MatMul3x34(Matrix3x3* res, Matrix3x3* a, Matrix3x3* b);
struct Matrix4x4         MatMul4x4(struct Matrix4x4 m1, struct Matrix4x4 m2);
struct Vec4f32           MatVecMul4x4(struct Matrix4x4 mat, struct Vec4f32 vec);
struct Vec3f32           MatVecMul3x3(struct Matrix3x3 m, struct Vec3f32 v);

struct Vec4f32           MatrixToVec4f32(struct Matrix4x4 m);
struct Vec3f32           MatrixToVec3f32(struct Matrix4x4 m);
struct Vec3i32           MatrixToVec3i32(struct Matrix4x4 m);

struct Matrix4x4         Vec3f32ToMatrix(struct Vec3f32 v);
struct Vec3f32           vectorSubtraction(struct Vec3f32 a, struct Vec3f32 b);
void                     normalizeVec3(struct Vec3f32* v);
f32                      dotProductVec3(struct Vec3f32 a, struct Vec3f32 b);

struct Vec3f32           barycentric3D(struct Vec2f32 v0, struct Vec2f32 v1, struct Vec2f32 v2, struct Vec2f32 point);

f32                      crossProduct2Df32(struct Vec2f32 v0, struct Vec2f32 v1, struct Vec2f32 point);
i32                      crossProduct2D(struct Vec2i32 v0, struct Vec2i32 v1, struct Vec2i32 point);
struct Vec3f32           crossProduct3D(struct Vec3f32 a, struct Vec3f32 b);
i32                      crossProduct3DVector(struct Vec3i32 a, struct Vec3i32 b, struct Vec3i32 p);

struct Vec3f32           randomPointOnSphere();
void                     getTransformationMatrix(Matrix3x3* res, f32 x, f32 y, f32 width, f32 height);
#endif
