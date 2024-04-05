#include "vector.h"
#include "common.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>

Vec3f32 randomPointOnSphere()
{

  float   u     = (float)rand() / (float)RAND_MAX;
  float   v     = (float)rand() / (float)RAND_MAX;
  float   theta = 2.f * PI * u;
  float   phi   = acos(2.f * v - 1.f);
  Vec3f32 res   = {sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi)};
  return res;
}

static inline f32 det2x2(f32 a00, f32 a01, f32 a10, f32 a11)
{
  return a00 * a11 - a01 * a10;
}
void debugVec3i32(Vec3i32 v)
{
  printf("%d %d %d\n", v.x, v.y, v.z);
}

void debugVec2f32(Vec2f32 v)
{
  printf("%lf\t%lf\n", v.x, v.y);
}
void debugVec3f32(Vec3f32 v)
{
  printf("%lf %lf %lf\n", v.x, v.y, v.z);
}

void debugVec4f32(Vec4f32 v)
{
  printf("%lf %lf %lf %lf\n", v.x, v.y, v.z, v.w);
}
static f32 det3x3(Matrix3x3 m)
{
  f32 d0 = m.m[0][0] * det2x2(m.m[1][1], m.m[1][2], m.m[2][1], m.m[2][2]);
  f32 d1 = m.m[0][1] * det2x2(m.m[1][0], m.m[1][2], m.m[2][0], m.m[2][2]);
  f32 d2 = m.m[0][2] * det2x2(m.m[1][0], m.m[1][1], m.m[2][0], m.m[2][1]);

  return (d0 - d1 + d2);
}

static f32 det4x4(Matrix4x4 m)
{
  Matrix3x3 m00 = {
      .i = {m.m[1][1], m.m[1][2], m.m[1][3]},
      .j = {m.m[2][1], m.m[2][2], m.m[2][3]},
      .k = {m.m[3][1], m.m[3][2], m.m[3][3]},
  };
  Matrix3x3 m01 = {
      .i = {m.m[1][0], m.m[1][2], m.m[1][3]},
      .j = {m.m[2][0], m.m[2][2], m.m[2][3]},
      .k = {m.m[3][0], m.m[3][2], m.m[3][3]},
  };
  Matrix3x3 m02 = {
      .i = {m.m[1][0], m.m[1][1], m.m[1][3]},
      .j = {m.m[2][0], m.m[2][1], m.m[2][3]},
      .k = {m.m[3][0], m.m[3][1], m.m[3][3]},
  };
  Matrix3x3 m03 = {
      .i = {m.m[1][0], m.m[1][1], m.m[1][2]},
      .j = {m.m[2][0], m.m[2][1], m.m[2][2]},
      .k = {m.m[3][0], m.m[3][1], m.m[3][2]},
  };

  f32 d0 = m.m[0][0] * det3x3(m00);
  f32 d1 = m.m[0][1] * det3x3(m01);
  f32 d2 = m.m[0][2] * det3x3(m02);
  f32 d3 = m.m[0][3] * det3x3(m03);
  return (d0 - d1 + d2 - d3);
}
static void scaleMatrix4x4(Matrix4x4* m, f32 scale)
{
  for (i32 i = 0; i < 4; i++)
  {
    for (i32 j = 0; j < 4; j++)
    {
      m->m[i][j] *= scale;
    }
  }
}

static void scaleMatrix3x3(Matrix3x3* m, f32 scale)
{
  for (i32 i = 0; i < 3; i++)
  {
    for (i32 j = 0; j < 3; j++)
    {
      m->m[i][j] *= scale;
    }
  }
}
void debugMatrix3x3(Matrix3x3 res)
{
  printf("%lf %lf %lf\n", res.i[0], res.i[1], res.i[2]);
  printf("%lf %lf %lf\n", res.j[0], res.j[1], res.j[2]);
  printf("%lf %lf %lf\n", res.k[0], res.k[1], res.k[2]);
}

void debugMatrix4x4(Matrix4x4 res)
{
  printf("%lf %lf %lf %lf\n", res.i[0], res.i[1], res.i[2], res.i[3]);
  printf("%lf %lf %lf %lf\n", res.j[0], res.j[1], res.j[2], res.j[3]);
  printf("%lf %lf %lf %lf\n", res.k[0], res.k[1], res.k[2], res.k[3]);
  printf("%lf %lf %lf %lf\n", res.l[0], res.l[1], res.l[2], res.l[3]);
}
Matrix4x4 invertMat4x4(Matrix4x4 m)
{
  Matrix3x3 m00 = {
      .i = {m.m[1][1], m.m[1][2], m.m[1][3]}, //
      .j = {m.m[2][1], m.m[2][2], m.m[2][3]}, //
      .k = {m.m[3][1], m.m[3][2], m.m[3][3]}  //
  };
  Matrix3x3 m01 = {
      .i = {m.m[1][0], m.m[1][2], m.m[1][3]}, //
      .j = {m.m[2][0], m.m[2][2], m.m[2][3]}, //
      .k = {m.m[3][0], m.m[3][2], m.m[3][3]}  //
  };
  Matrix3x3 m02 = {
      .i = {m.m[1][0], m.m[1][1], m.m[1][3]}, //
      .j = {m.m[2][0], m.m[2][1], m.m[2][3]}, //
      .k = {m.m[3][0], m.m[3][1], m.m[3][3]}  //
  };
  Matrix3x3 m03 = {
      .i = {m.m[1][0], m.m[1][1], m.m[1][2]}, //
      .j = {m.m[2][0], m.m[2][1], m.m[2][2]}, //
      .k = {m.m[3][0], m.m[3][1], m.m[3][2]}  //
  };

  Matrix3x3 m10 = {
      .i = {m.m[0][1], m.m[0][2], m.m[0][3]}, //
      .j = {m.m[2][1], m.m[2][2], m.m[2][3]}, //
      .k = {m.m[3][1], m.m[3][2], m.m[3][3]}  //
  };
  Matrix3x3 m11 = {
      .i = {m.m[0][0], m.m[0][2], m.m[0][3]}, //
      .j = {m.m[2][0], m.m[2][2], m.m[2][3]}, //
      .k = {m.m[3][0], m.m[3][2], m.m[3][3]}  //
  };
  Matrix3x3 m12 = {
      .i = {m.m[0][0], m.m[0][1], m.m[0][3]}, //
      .j = {m.m[2][0], m.m[2][1], m.m[2][3]}, //
      .k = {m.m[3][0], m.m[3][1], m.m[3][3]}  //
  };
  Matrix3x3 m13 = {
      .i = {m.m[0][0], m.m[0][1], m.m[0][2]}, //
      .j = {m.m[2][0], m.m[2][1], m.m[2][2]}, //
      .k = {m.m[3][0], m.m[3][1], m.m[3][2]}  //
  };

  Matrix3x3 m20 = {
      .i = {m.m[0][1], m.m[0][2], m.m[0][3]}, //
      .j = {m.m[1][1], m.m[1][2], m.m[1][3]}, //
      .k = {m.m[3][1], m.m[3][2], m.m[3][3]}  //
  };

  Matrix3x3 m21 = {
      .i = {m.m[0][0], m.m[0][2], m.m[0][3]}, //
      .j = {m.m[1][0], m.m[1][2], m.m[1][3]}, //
      .k = {m.m[3][0], m.m[3][2], m.m[3][3]}  //
  };
  Matrix3x3 m22 = {
      .i = {m.m[0][0], m.m[0][1], m.m[0][3]}, //
      .j = {m.m[1][0], m.m[1][1], m.m[1][3]}, //
      .k = {m.m[3][0], m.m[3][1], m.m[3][3]}  //
  };
  Matrix3x3 m23 = {
      .i = {m.m[0][0], m.m[0][1], m.m[0][2]}, //
      .j = {m.m[1][0], m.m[1][1], m.m[1][2]}, //
      .k = {m.m[3][0], m.m[3][1], m.m[3][2]}  //
  };
  Matrix3x3 m30 = {
      .i = {m.m[0][1], m.m[0][2], m.m[0][3]}, //
      .j = {m.m[1][1], m.m[1][2], m.m[1][3]}, //
      .k = {m.m[2][1], m.m[2][2], m.m[2][3]}  //
  };
  Matrix3x3 m31 = {
      .i = {m.m[0][0], m.m[0][2], m.m[0][3]}, //
      .j = {m.m[1][0], m.m[1][2], m.m[1][3]}, //
      .k = {m.m[2][0], m.m[2][2], m.m[2][3]}  //
  };
  Matrix3x3 m32 = {
      .i = {m.m[0][0], m.m[0][1], m.m[0][3]}, //
      .j = {m.m[1][0], m.m[1][1], m.m[1][3]}, //
      .k = {m.m[2][0], m.m[2][1], m.m[2][3]}  //
  };
  Matrix3x3 m33 = {
      .i = {m.m[0][0], m.m[0][1], m.m[0][2]}, //
      .j = {m.m[1][0], m.m[1][1], m.m[1][2]}, //
      .k = {m.m[2][0], m.m[2][1], m.m[2][2]}  //
  };
  Matrix4x4 res = {
      .i = { det3x3(m00), -det3x3(m01),  det3x3(m02), -det3x3(m03)},
      .j = {-det3x3(m10),  det3x3(m11), -det3x3(m12),  det3x3(m13)},
      .k = { det3x3(m20), -det3x3(m21),  det3x3(m22), -det3x3(m23)},
      .l = {-det3x3(m30),  det3x3(m31), -det3x3(m32),  det3x3(m33)},
  };
  // debugMatrix4x4(res);

  f32 det   = det4x4(m);
  f32 scale = 1.0f / det;
  scaleMatrix4x4(&res, scale);
  return res;
}
Vec2f32 ProjectVec3ToVec2(Vec3f32 v)
{
  Vec2f32 res = {v.x / v.z, v.y / v.z};
  return res;
}
Vec3f32 ProjectVec4ToVec3(Vec4f32 v)
{
  Vec3f32 res = {v.x / v.w, v.y / v.w, v.z / v.w};
  return res;
}

Vec4f32 MatrixToVec4f32(Matrix4x4 m)
{
  Vec4f32 res = {m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0]};
  return res;
}
Vec3i32 MatrixToVec3i32(Matrix4x4 m)
{
  f32     scale = m.m[3][0];
  Vec3i32 res;
  res.x = m.m[0][0] / scale;
  res.y = m.m[1][0] / scale;
  res.z = m.m[2][0] / scale;
  return res;
}
void clearMat3x3(Matrix3x3* m)
{
  for (i32 i = 0; i < 9; i++)
  {
    m->a[i] = 0;
  }
}
void getTransformationMatrix(Matrix3x3* res, f32 x, f32 y, f32 width, f32 height)
{

  f32       transformX   = x * 0.01f;
  f32       transformY   = y * 0.01f;
  Matrix3x3 translationM = {
      1,          0,          0, //
      0,          1,          0, //
      transformX, transformY, 1  //
  };

  float     scaleX = width * 0.01f;
  float     scaleY = height * 0.01f;

  Matrix3x3 scaleM = {
      scaleX, 0,      0, //
      0,      scaleY, 0, //
      0,      0,      1  //
  };
  clearMat3x3(res);
  MatMul3x3(res, &translationM, &scaleM);
}

Matrix3x3 invertMat3x3(Matrix3x3 a)
{
  Matrix3x3 res = {
      det2x2(a.j[1], a.j[2], a.k[1], a.k[2]), det2x2(a.i[2], a.i[1], a.k[2], a.k[1]), det2x2(a.i[1], a.i[2], a.j[1], a.j[2]), //
      det2x2(a.j[2], a.j[0], a.k[2], a.k[0]), det2x2(a.i[0], a.i[2], a.k[0], a.k[2]), det2x2(a.i[2], a.i[0], a.j[2], a.j[0]), //
      det2x2(a.j[0], a.j[1], a.k[0], a.k[1]), det2x2(a.i[1], a.i[0], a.k[1], a.k[0]), det2x2(a.i[0], a.i[1], a.j[0], a.j[1]), //
  };
  f32 scale = 1.0f / det3x3(a);
  scaleMatrix3x3(&res, scale);
  return res;
}

Vec3f32 MatVecMul3x3(Matrix3x3 m, Vec3f32 v)
{
  Vec3f32 res = {
      .x = m.i[0] * v.x + m.i[1] * v.y + m.i[2] * v.z,
      .y = m.j[0] * v.x + m.j[1] * v.y + m.j[2] * v.z,
      .z = m.k[0] * v.x + m.k[1] * v.y + m.k[2] * v.z,
  };
  return res;
}
Vec3f32 VecMul3f32(Vec3f32 a, Vec3f32 b)
{
  Vec3f32 res = {
      a.x * b.x, //
      a.y * b.y, //
      a.z * b.z  //
  };

  return res;
}

Vec4f32 MatVecMul4x4(Matrix4x4 mat, Vec4f32 vec)
{
  Vec4f32 res = {
      mat.m[0][0] * vec.x + mat.m[0][1] * vec.y + mat.m[0][2] * vec.z + mat.m[0][3] * vec.w, //
      mat.m[1][0] * vec.x + mat.m[1][1] * vec.y + mat.m[1][2] * vec.z + mat.m[1][3] * vec.w, //
      mat.m[2][0] * vec.x + mat.m[2][1] * vec.y + mat.m[2][2] * vec.z + mat.m[2][3] * vec.w, //
      mat.m[3][0] * vec.x + mat.m[3][1] * vec.y + mat.m[3][2] * vec.z + mat.m[3][3] * vec.w, //
                                                                                             //
  };
  return res;
}

Matrix4x4 lookAt(Vec3f32 eye, Vec3f32 center, Vec3f32 up)
{

  Vec3f32 z = {eye.x - center.x, eye.y - center.y, eye.z - center.z};
  normalizeVec3(&z);

  Vec3f32 x = crossProduct3D(up, z);
  normalizeVec3(&x);

  Vec3f32 y = crossProduct3D(z, x);
  normalizeVec3(&y);

  Matrix4x4 minV;
  buildIdentityMatrix4x4(&minV);

  Matrix4x4 tr;
  buildIdentityMatrix4x4(&tr);

  for (i32 i = 0; i < 3; i++)
  {
    minV.m[0][i] = x.pos[i];
    minV.m[1][i] = y.pos[i];
    minV.m[2][i] = z.pos[i];
    tr.m[i][3]   = -center.pos[i];
  }
  return MatMul4x4(minV, tr);
}

Matrix3x3 transposeMat3x3(Matrix3x3 m)
{
  Matrix3x3 res = {
      .i = {m.i[0], m.j[0], m.k[0]}, //
      .j = {m.i[1], m.j[1], m.k[1]}, //
      .k = {m.i[2], m.j[2], m.k[2]}, //
  };
  return res;
}
Matrix4x4 transposeMat4x4(Matrix4x4 m)
{
  Matrix4x4 res = {
      .i = {m.i[0], m.j[0], m.k[0], m.l[0]},
      .j = {m.i[1], m.j[1], m.k[1], m.l[1]},
      .k = {m.i[2], m.j[2], m.k[2], m.l[2]},
      .l = {m.i[3], m.j[3], m.k[3], m.l[3]},
  };

  return res;
}


void MatMul3x3(Matrix3x3* res, Matrix3x3* a, Matrix3x3* b)
{
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      for (int k = 0; k < 3; k++)
      {
        res->a[i + j * 3] += a->a[i + k * 3] * b->a[k + j * 3];
      }
    }
  }
}

Matrix4x4 MatMul4x4(Matrix4x4 a, Matrix4x4 b)
{
  f32       a00 = a.m[0][0], a01 = a.m[0][1], a02 = a.m[0][2], a03 = a.m[0][3];
  f32       a10 = a.m[1][0], a11 = a.m[1][1], a12 = a.m[1][2], a13 = a.m[1][3];
  f32       a20 = a.m[2][0], a21 = a.m[2][1], a22 = a.m[2][2], a23 = a.m[2][3];
  f32       a30 = a.m[3][0], a31 = a.m[3][1], a32 = a.m[3][2], a33 = a.m[3][3];

  f32       b00 = b.m[0][0], b01 = b.m[0][1], b02 = b.m[0][2], b03 = b.m[0][3];
  f32       b10 = b.m[1][0], b11 = b.m[1][1], b12 = b.m[1][2], b13 = b.m[1][3];
  f32       b20 = b.m[2][0], b21 = b.m[2][1], b22 = b.m[2][2], b23 = b.m[2][3];
  f32       b30 = b.m[3][0], b31 = b.m[3][1], b32 = b.m[3][2], b33 = b.m[3][3];

  Matrix4x4 m;
  m.m[0][0] = a00 * b00 + a01 * b10 + a02 * b20 + a03 * b30;
  m.m[0][1] = a00 * b01 + a01 * b11 + a02 * b21 + a03 * b31;
  m.m[0][2] = a00 * b02 + a01 * b12 + a02 * b22 + a03 * b32;
  m.m[0][3] = a00 * b03 + a01 * b13 + a02 * b23 + a03 * b33;

  m.m[1][0] = a10 * b00 + a11 * b10 + a12 * b20 + a13 * b30;
  m.m[1][1] = a10 * b01 + a11 * b11 + a12 * b21 + a13 * b31;
  m.m[1][2] = a10 * b02 + a11 * b12 + a12 * b22 + a13 * b32;
  m.m[1][3] = a10 * b03 + a11 * b13 + a12 * b23 + a13 * b33;

  m.m[2][0] = a20 * b00 + a21 * b10 + a22 * b20 + a23 * b30;
  m.m[2][1] = a20 * b01 + a21 * b11 + a22 * b21 + a23 * b31;
  m.m[2][2] = a20 * b02 + a21 * b12 + a22 * b22 + a23 * b32;
  m.m[2][3] = a20 * b03 + a21 * b13 + a22 * b23 + a23 * b33;

  m.m[3][0] = a30 * b00 + a31 * b10 + a32 * b20 + a33 * b30;
  m.m[3][1] = a30 * b01 + a31 * b11 + a32 * b21 + a33 * b31;
  m.m[3][2] = a30 * b02 + a31 * b12 + a32 * b22 + a33 * b32;
  m.m[3][3] = a30 * b03 + a31 * b13 + a32 * b23 + a33 * b33;

  return m;
}

Matrix4x4 Vec3f32ToMatrix(Vec3f32 v)
{
  Matrix4x4 m;
  buildIdentityMatrix4x4(&m);
  m.m[0][0] = v.x;
  m.m[1][0] = v.y;
  m.m[2][0] = v.z;
  m.m[3][0] = 1.0f;

  return m;
}

Vec3f32 MatrixToVec3f32(Matrix4x4 m)
{
  Vec3f32 v = {m.m[0][0] / m.m[3][0], m.m[1][0] / m.m[3][0], m.m[2][0] / m.m[3][0]};

  return v;
}
void buildViewportMatrix4x4(Matrix4x4* m, i32 x, i32 y, i32 w, i32 h, i32 depth)
{
  m->m[0][3] = x + w / 2.0f;
  m->m[1][3] = y + h / 2.0f;
  m->m[2][3] = depth / 2.0f;

  m->m[0][0] = w / 2.0f;
  m->m[1][1] = h / 2.0f;
  m->m[2][2] = depth / 2.0f;
}
void buildIdentityMatrix4x4(Matrix4x4* m)
{
  m->m[0][0] = 1;
  m->m[0][1] = 0;
  m->m[0][2] = 0;
  m->m[0][3] = 0;

  m->m[1][0] = 0;
  m->m[1][1] = 1;
  m->m[1][2] = 0;
  m->m[1][3] = 0;

  m->m[2][0] = 0;
  m->m[2][1] = 0;
  m->m[2][2] = 1;
  m->m[2][3] = 0;

  m->m[3][0] = 0;
  m->m[3][1] = 0;
  m->m[3][2] = 0;
  m->m[3][3] = 1;
}

i32 crossProduct2D(Vec2i32 v0, Vec2i32 v1, Vec2i32 point)
{
  Vec2i32 v0v1 = {v1.x - v0.x, v1.y - v0.y};
  Vec2i32 v0p  = {point.x - v0.x, point.y - v0.y};

  return v0v1.x * v0p.y - v0v1.y * v0p.x;
}

f32 crossProduct2Df32(Vec2f32 v0, Vec2f32 v1, Vec2f32 point)
{
  Vec2f32 v0v1 = {v1.x - v0.x, v1.y - v0.y};
  Vec2f32 v0p  = {point.x - v0.x, point.y - v0.y};

  return v0v1.x * v0p.y - v0v1.y * v0p.x;
}
Vec3f32 barycentric3D(Vec2f32 v0, Vec2f32 v1, Vec2f32 v2, Vec2f32 point)
{
  f32     total = crossProduct2Df32(v0, v1, v2);
  Vec3f32 b     = {
      crossProduct2Df32(v0, v2, point) / total, //
      crossProduct2Df32(v1, v0, point) / total, //
      crossProduct2Df32(v2, v0, point) / total  //
  };

  return b;
}

Vec3f32 vectorSubtraction(Vec3f32 a, Vec3f32 b)
{
  Vec3f32 res = {
      a.x - b.x, //
      a.y - b.y, //
      a.z - b.z  //
  };

  return res;
}

void normalizeVec3(Vec3f32* v)
{
  f32 len = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
  v->x /= len;
  v->y /= len;
  v->z /= len;
}

f32 dotProductVec3(Vec3f32 a, Vec3f32 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3f32 crossProduct3D(Vec3f32 a, Vec3f32 b)
{
  Vec3f32 res = {
      a.y * b.z - a.z * b.y, //
      a.z * b.x - a.x * b.z, //
      a.x * b.y - a.y * b.x  //
  };

  return res;
}
