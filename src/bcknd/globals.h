#pragma once

#include "LWmath.h"

typedef unsigned char uchar;

typedef int32_t  i32;
typedef int64_t  i64;
typedef uint32_t ui32;
typedef uint64_t ui64;

typedef size_t   arch;

typedef float    f32;
typedef double   f64;

typedef Vector2<int>    ivec2;
typedef Vector2<float>  fvec2;
typedef Vector2<double> dvec2;

typedef Vector3<int>    ivec3;
typedef Vector3<float>  fvec3;
typedef Vector3<double> dvec3;

typedef Vector4<int>    ivec4;
typedef Vector4<float>  fvec4;
typedef Vector4<double> dvec4;

typedef Matrix4<int>    imat4;
typedef Matrix4<float>  fmat4;
typedef Matrix4<double> dmat4;

#define VK_CHECK( expr )  if ( (expr) != VK_SUCCESS ) { std::cout << "Vulkan Error evaluating \" "<< #expr << " \" " << std::endl; }
#define VK_CHECK_EXTENDED( expr, msg ) if ( (expr) != VK_SUCCESS ) { std::cout << "Vulkan Error evaluating \" "<< #expr << " \" " << "Info: " << msg << std::endl; }
 
