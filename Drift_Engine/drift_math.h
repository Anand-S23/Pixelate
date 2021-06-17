#ifndef DRIFT_MATH_H
#define DRIFT_MATH_H

#include <math.h>

// Common

#define PIf                               3.1415926535897f
#define TAU                               6.28318530718f
#define PI_OVER_TWO                       1.57079632679f
#define ONE_OVER_SQUARE_ROOT_OF_TWO_PI    0.3989422804
#define ONE_OVER_SQUARE_ROOT_OF_TWO_PIf   0.3989422804f
#define EULERS_NUMBERf                    2.7182818284590452353602874713527f
#define DEG_TO_RAD                        0.0174532925f
#define RAD_TO_DEG                        57.2958f

#define Bytes(n)     (n)
#define Kilobytes(n) (Bytes(n)*1024LL)
#define Megabytes(n) (Kilobytes(n)*1024LL)
#define Gigabytes(n) (Megabytes(n)*1024LL)
#define Terabytes(n) (Gigabytes(n)*1024LL)

#define ArraySize(arr)        (sizeof(arr) / sizeof((arr)[0]))
#define Min(a, b)             ((a) < (b) ? (a) : (b))
#define Max(a, b)             ((a) > (b) ? (a) : (b))
#define AbsoluteValue(a)      ((a) > 0 ? (a) : -(a))
#define Lerp(a, b, t)         (a + (b - a) * t)
#define Clamp01(val)          Min(1.f, Max(0.f, (val)))
#define Clamp(min, max, val)  Min((max), Max((min), (val)))
#define Square(a)             ((a) * (a))

// Vector Math

typedef union v2 
{
    struct
    {
        float x; 
        float y; 
    }; 

    struct 
    {
        float width; 
        float height;
    };
    
} v2;

typedef union v3 
{
    struct 
    {
        float x;
        float y;
        float z;
    };
    
    struct 
    {
        float r;
        float g;
        float b;
    };
    
    float elements[3];
} v3;

typedef union v4 
{
    struct 
    {
        float x;
        float y;
        union 
        {
            struct 
            {
                float z;
                union
                {
                    float w;
                    float radius;
                };
            };

            struct 
            {
                float width;
                float height;
            };
        };
    };
    
    struct 
    {
        float r;
        float g;
        float b;
        float a;
    };
    
    float elements[4];
} v4;

static v2 V2Init(float x, float y)
{
    v2 v = { x, y };
    return v;
}
#define v2(x, y) V2Init(x, y)

static v3 V3Init(float x, float y, float z)
{
    v3 v = { x, y, z };
    return v; 
}
#define v3(x, y, z) V3Init(x, y, z)

static v4 V4Init(float x, float y, float z, float w)
{
    v4 v = { x, y, z, w }; 
    return v; 
}
#define v4(x, y, z, w) V4Init(x, y, z, w)

static v2 V2Scalar(v2 vec, float s)
{
    vec.x *= s;
    vec.y *= s;

    return vec;
}

static v3 V3Scalar(v3 vec, float s)
{
    vec.x *= s;
    vec.y *= s;
    vec.z *= s;

    return vec;
}

static v4 V4Scalar(v4 vec, float s)
{
    vec.x *= s;
    vec.y *= s;
    vec.z *= s;
    vec.w *= s;

    return vec;
}

static v2 V2Add(v2 vec1, v2 vec2)
{
    vec1.x += vec2.x;
    vec1.y += vec2.y;

    return vec1;
}

static v3 V3Add(v3 vec1, v3 vec2)
{
    vec1.x += vec2.x;
    vec1.y += vec2.y;
    vec1.z += vec2.z;

    return vec1;
}

static v4 V4Add(v4 vec1, v4 vec2)
{
    vec1.x += vec2.x;
    vec1.y += vec2.y;
    vec1.z += vec2.z;
    vec1.w += vec2.w;

    return vec1;
}

static v2 V2Subtract(v2 vec1, v2 vec2)
{
    vec1.x -= vec2.x;
    vec1.y -= vec2.y;

    return vec1;
}

static v3 V3Subtract(v3 vec1, v3 vec2)
{
    vec1.x -= vec2.x;
    vec1.y -= vec2.y;
    vec1.z -= vec2.z;

    return vec1;
}

static v4 V4Subtract(v4 vec1, v4 vec2)
{
    vec1.x -= vec2.x;
    vec1.y -= vec2.y;
    vec1.z -= vec2.z;
    vec1.w -= vec2.w;

    return vec1;
}

static float V2Magnitude(v2 vec)
{
    return (float)sqrt(vec.x * vec.x + vec.y * vec.y);
}

static v2 V2Normalize(v2 vec)
{
    float scalar = 1.f / V2Magnitude(vec);
    return V2Scalar(vec, scalar);
}

static float V3Magnitude(v3 vec)
{
    return (float)sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

static v3 V3Normalize(v3 vec)
{
    float scalar = 1.f / V3Magnitude(vec);
    return V3Scalar(vec, scalar);
}

static float V4Magnitude(v4 vec)
{
    return (float)sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
}

static v4 V4Normalize(v4 vec)
{
    float scalar = 1.f / V4Magnitude(vec);
    return V4Scalar(vec, scalar);
}

static float V2Dot(v2 vec1, v2 vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y; 
}

static float V3Dot(v3 vec1, v3 vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z; 
}

static float V4Dot(v4 vec1, v4 vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z + vec1.w * vec2.w;
}

static v3 V3Cross(v3 vec1, v3 vec2)
{
    return v3(vec1.y * vec2.z - vec1.z * vec2.y,
              vec1.z * vec2.x - vec1.x * vec2.z,
              vec1.x * vec2.y - vec1.y * vec2.x);
}

typedef struct matrix4f
{
    float elements[4][4];
} matrix4f;

static matrix4f IdentityMatrix()
{
    matrix4f m = {0};
    {
        m.elements[0][0] = 1.f;
        m.elements[1][1] = 1.f;
        m.elements[2][2] = 1.f;
        m.elements[3][3] = 1.f;
    }

    return m;
}

static matrix4f Matrix4f(f32 e00, f32 e01, f32 e02, f32 e03,
                         f32 e10, f32 e11, f32 e12, f32 e13,
                         f32 e20, f32 e21, f32 e22, f32 e23,
                         f32 e30, f32 e31, f32 e32, f32 e33)
{
    matrix4f m = {0};
    {
        m.elements[0][0] = e00;
        m.elements[0][1] = e01;
        m.elements[0][2] = e02;
        m.elements[0][3] = e03;

        m.elements[1][0] = e10;
        m.elements[1][1] = e11;
        m.elements[1][2] = e12;
        m.elements[1][3] = e13;

        m.elements[2][0] = e20;
        m.elements[2][1] = e21;
        m.elements[2][2] = e22;
        m.elements[2][3] = e23;

        m.elements[3][0] = e30;
        m.elements[3][1] = e31;
        m.elements[3][2] = e32;
        m.elements[3][3] = e33;
    }

    return m;
}

static matrix4f Matrix4fFromV4(v4 vec1, v4 vec2, v4 vec3, v4 vec4)
{
    matrix4f m = {0};
   {
        m.elements[0][0] = vec1.x;
        m.elements[0][1] = vec2.x;
        m.elements[0][2] = vec3.x;
        m.elements[0][3] = vec4.x;

        m.elements[1][0] = vec1.y;
        m.elements[1][1] = vec2.y;
        m.elements[1][2] = vec3.y;
        m.elements[1][3] = vec4.y;

        m.elements[2][0] = vec1.z;
        m.elements[2][1] = vec2.z;
        m.elements[2][2] = vec3.z;
        m.elements[2][3] = vec4.z;

        m.elements[3][0] = vec1.w;
        m.elements[3][1] = vec2.w;
        m.elements[3][2] = vec3.w;
        m.elements[3][3] = vec4.w;
    }

    return m;
}

// TODO: Invere, Multiply

static matrix4f Matrix4fTranspose(matrix4f mat)
{
    matrix4f m = {0};
    for (int j = 0; j < 4; ++j)
    {
        for (int i = 0; i < 4; ++i)
        {
            m.elements[j][i] = mat.elements[i][j];
        }
    }

    return m;
}

static matrix4f Matrix4fAdd(matrix4f m1, matrix4f m2)
{
    for (int j = 0; j < 4; ++j)
    {
        for (int i = 0; i < 4; ++i)
        {
            m1.elements[j][i] += m2.elements[j][i];
        }
    }

    return m1;
}

static matrix4f Matrix4fSubtract(matrix4f m1, matrix4f m2)
{
    for (int j = 0; j < 4; ++j)
    {
        for (int i = 0; i < 4; ++i)
        {
            m1.elements[j][i] -= m2.elements[j][i];
        }
    }

    return m1;
}

static matrix4f Matrix4fScalar(matrix4f m, f32 s)
{

    for (int j = 0; j < 4; ++j)
    {
        for (int i = 0; i < 4; ++i)
        {
            m.elements[j][i] *= s;
        }
    }

    return m;
}

static v4 Matrix4fVecMultiply(matrix4f m, v4 v)
{
    return v4(m.elements[0][0] * v.x + m.elements[0][1] * v.y + 
              m.elements[0][2] * v.z + m.elements[0][3] * v.w,

              m.elements[1][0] * v.x + m.elements[1][1] * v.y + 
              m.elements[1][2] * v.z + m.elements[1][3] * v.w,

              m.elements[2][0] * v.x + m.elements[2][1] * v.y + 
              m.elements[2][2] * v.z + m.elements[2][3] * v.w,

              m.elements[3][0] * v.x + m.elements[3][1] * v.y + 
              m.elements[3][2] * v.z + m.elements[3][3] * v.w);
}

static matrix4f Matrix4fMultiply(matrix4f m1, matrix4f m2)
{
    f32 e00 = m1.elements[0][0] * m2.elements[0][0] +
              m1.elements[0][1] * m2.elements[1][0] +
              m1.elements[0][2] * m2.elements[2][0] +
              m1.elements[0][3] * m2.elements[3][0];

    f32 e01 = m1.elements[0][0] * m2.elements[0][1] +
              m1.elements[0][1] * m2.elements[1][1] +
              m1.elements[0][2] * m2.elements[2][1] +
              m1.elements[0][3] * m2.elements[3][1];

    f32 e02 = m1.elements[0][0] * m2.elements[0][2] +
              m1.elements[0][1] * m2.elements[1][2] +
              m1.elements[0][2] * m2.elements[2][2] +
              m1.elements[0][3] * m2.elements[3][2];

    f32 e03 = m1.elements[0][0] * m2.elements[0][3] +
              m1.elements[0][1] * m2.elements[1][3] +
              m1.elements[0][2] * m2.elements[2][3] +
              m1.elements[0][3] * m2.elements[3][3];

    f32 e10 = m1.elements[1][0] * m2.elements[0][0] +
              m1.elements[1][1] * m2.elements[1][0] +
              m1.elements[1][2] * m2.elements[2][0] +
              m1.elements[1][3] * m2.elements[3][0];

    f32 e11 = m1.elements[1][0] * m2.elements[0][1] +
              m1.elements[1][1] * m2.elements[1][1] +
              m1.elements[1][2] * m2.elements[2][1] +
              m1.elements[1][3] * m2.elements[3][1];

    f32 e12 = m1.elements[1][0] * m2.elements[0][2] +
              m1.elements[1][1] * m2.elements[1][2] +
              m1.elements[1][2] * m2.elements[2][2] +
              m1.elements[1][3] * m2.elements[3][2];

    f32 e13 = m1.elements[1][0] * m2.elements[0][3] +
              m1.elements[1][1] * m2.elements[1][3] +
              m1.elements[1][2] * m2.elements[2][3] +
              m1.elements[1][3] * m2.elements[3][3];

    f32 e20 = m1.elements[2][0] * m2.elements[0][0] +
              m1.elements[2][1] * m2.elements[1][0] +
              m1.elements[2][2] * m2.elements[2][0] +
              m1.elements[2][3] * m2.elements[3][0];

    f32 e21 = m1.elements[2][0] * m2.elements[0][1] +
              m1.elements[2][1] * m2.elements[1][1] +
              m1.elements[2][2] * m2.elements[2][1] +
              m1.elements[2][3] * m2.elements[3][1];

    f32 e22 = m1.elements[2][0] * m2.elements[0][2] +
              m1.elements[2][1] * m2.elements[1][2] +
              m1.elements[2][2] * m2.elements[2][2] +
              m1.elements[2][3] * m2.elements[3][2];

    f32 e23 = m1.elements[2][0] * m2.elements[0][3] +
              m1.elements[2][1] * m2.elements[1][3] +
              m1.elements[2][2] * m2.elements[2][3] +
              m1.elements[2][3] * m2.elements[3][3];

    f32 e30 = m1.elements[3][0] * m2.elements[0][0] +
              m1.elements[3][1] * m2.elements[1][0] +
              m1.elements[3][2] * m2.elements[2][0] +
              m1.elements[3][3] * m2.elements[3][0];

    f32 e31 = m1.elements[3][0] * m2.elements[0][1] +
              m1.elements[3][1] * m2.elements[1][1] +
              m1.elements[3][2] * m2.elements[2][1] +
              m1.elements[3][3] * m2.elements[3][1];

    f32 e32 = m1.elements[3][0] * m2.elements[0][2] +
              m1.elements[3][1] * m2.elements[1][2] +
              m1.elements[3][2] * m2.elements[2][2] +
              m1.elements[3][3] * m2.elements[3][2];

    f32 e33 = m1.elements[3][0] * m2.elements[0][3] +
              m1.elements[3][1] * m2.elements[1][3] +
              m1.elements[3][2] * m2.elements[2][3] +
              m1.elements[3][3] * m2.elements[3][3];

    return Matrix4f(e00, e01, e02, e03, e10, e11, e12, e13,
                    e20, e21, e22, e23, e30, e31, e32, e33);
}

static matrix4f OrthographicMatrix(f32 left, f32 right,
                                   f32 bottom, f32 top,
                                   f32 near_plane, f32 far_plane)
{
    matrix4f m = {0};
    {
        m.elements[0][0] = 2.f / (right - left);
        m.elements[1][1] = 2.f / (top - bottom);
        m.elements[2][2] = -2.f / (far_plane - near_plane);
        m.elements[3][3] = 1.f;
        m.elements[3][0] = (left + right) / (left - right);
        m.elements[3][1] = (bottom + top) / (bottom - top);
        m.elements[3][2] = (far_plane + near_plane) / (near_plane - far_plane);
    }

    return m;
}

/*
static void TranslateMatrix(matrix4f *m, v3 translation)
{
    m->elements[0][3] = translation.x;
    m->elements[1][3] = translation.y;
    m->elements[2][3] = translation.z;
}

static void ScaleMatrix(matrix4f *m, v3 scale)
{
    m->elements[0][0] = scale.x;
    m->elements[1][1] = scale.y;
    m->elements[2][2] = scale.z;
}
*/

// TODO: Qauterions

#endif
