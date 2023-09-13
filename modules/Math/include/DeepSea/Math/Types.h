/*
 * Copyright 2016-2023 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Math/SIMD/SIMD.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Workaround for system headers included before this file that #define half the English language.
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/Math library.
 */

/**
 * @brief Enum for the options that can be applied when creating a projection matrix.
 */
typedef enum dsProjectionMatrixOptions
{
	dsProjectionMatrixOptions_None = 0, ///< No options applied to the matrix.
	/**
	 * Use [0, 1] for the Z coordinate range instead of [-1, 1].
	 */
	dsProjectionMatrixOptions_HalfZRange = 0x1,
	dsProjectionMatrixOptions_InvertY = 0x2, ///< Invert the Y coordinate of the matrix.
	dsProjectionMatrixOptions_InvertZ = 0x4  ///< Invert the Z coordinate of the matrix.
} dsProjectionMatrixOptions;

/**
 * @brief Structure for a 2D vector holding floats.
 *
 * This can be accessed using cartesian coordinates (x, y), texture coordinates (s, t), color
 * channels (r, g), or an array of values.
 *
 * @remark When bracket initializing, use two brackets. (i.e. {{x, y}})
 * @see Vector2.h
 */
typedef union dsVector2f
{
	/**
	 * @brief Array of the vector values.
	 */
	float values[2];

	struct
	{
		/**
		 * @brief The x coordinate.
		 */
		float x;

		/**
		 * @brief The y coordinate.
		 */
		float y;
	};
	struct
	{
		/**
		 * @brief The s coordinate.
		 */
		float s;

		/**
		 * @brief The t coordinate.
		 */
		float t;
	};
	struct
	{
		/**
		 * @brief The red component.
		 */
		float r;

		/**
		 * @brief The green component.
		 */
		float g;
	};
} dsVector2f;

/**
 * @brief Structure for a 2D vector holding doubles.
 *
 * This can be accessed using cartesian coordinates (x, y), texture coordinates (s, t), color
 * channels (r, g), or an array of values.
 *
 * @remark When bracket initializing, use two brackets. (i.e. {{x, y}})
 * @see Vector2.h
 */
typedef union dsVector2d
{
	/**
	 * @brief Array of the vector values.
	 */
	double values[2];

	/**
	 * @brief SIMD value when supported.
	 */
#if DS_HAS_SIMD
	dsSIMD2d simd;
#endif

	struct
	{
		/**
		 * @brief The x coordinate.
		 */
		double x;

		/**
		 * @brief The y coordinate.
		 */
		double y;
	};
	struct
	{
		/**
		 * @brief The s coordinate.
		 */
		double s;

		/**
		 * @brief The t coordinate.
		 */
		double t;
	};
	struct
	{
		/**
		 * @brief The red component.
		 */
		double r;

		/**
		 * @brief The green component.
		 */
		double g;
	};
} dsVector2d;

/**
 * @brief Structure for a 2D vector holding ints.
 *
 * This can be accessed using cartesian coordinates (x, y), texture coordinates (s, t), color
 * channels (r, g), or an array of values.
 *
 * @remark When bracket initializing, use two brackets. (i.e. {{x, y}})
 * @see Vector2.h
 */
typedef union dsVector2i
{
	/**
	 * @brief Array of the vector values.
	 */
	int values[2];

	struct
	{
		/**
		 * @brief The x coordinate.
		 */
		int x;

		/**
		 * @brief The y coordinate.
		 */
		int y;
	};
	struct
	{
		/**
		 * @brief The s coordinate.
		 */
		int s;

		/**
		 * @brief The t coordinate.
		 */
		int t;
	};
	struct
	{
		/**
		 * @brief The red component.
		 */
		int r;

		/**
		 * @brief The green component.
		 */
		int g;
	};
} dsVector2i;

/**
 * @brief Structure for a 2D vector holding 64-bit longs.
 *
 * This can be accessed using cartesian coordinates (x, y), texture coordinates (s, t), color
 * channels (r, g), or an array of values.
 *
 * @remark When bracket initializing, use two brackets. (i.e. {{x, y}})
 * @see Vector2.h
 */
typedef union dsVector2l
{
	/**
	 * @brief Array of the vector values.
	 */
	long long values[2];

	/**
	 * @brief SIMD value when supported.
	 */
#if DS_HAS_SIMD
	dsSIMD2db simd;
#endif

	struct
	{
		/**
		 * @brief The x coordinate.
		 */
		long long x;

		/**
		 * @brief The y coordinate.
		 */
		long long y;
	};
	struct
	{
		/**
		 * @brief The s coordinate.
		 */
		long long s;

		/**
		 * @brief The t coordinate.
		 */
		long long t;
	};
	struct
	{
		/**
		 * @brief The red component.
		 */
		long long r;

		/**
		 * @brief The green component.
		 */
		long long g;
	};
} dsVector2l;

/**
 * @brief Structure for a 3D vector holding floats.
 *
 * This can be accessed using cartesian coordinates (x, y, z), texture coordinates (s, t, p), color
 * channels (r, g, b), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 *
 * @remark When bracket initializing, use two brackets. (i.e. {{x, y, z}})
 * @see Vector3.h
 */
typedef union dsVector3f
{
	/**
	 * @brief Array of the vector values.
	 */
	float values[3];

	struct
	{
		/**
		 * @brief The x coordinate.
		 */
		float x;

		/**
		 * @brief The y coordinate.
		 */
		float y;

		/**
		 * @brief The z coordinate.
		 */
		float z;
	};
	struct
	{
		/**
		 * @brief The s coordinate.
		 */
		float s;

		/**
		 * @brief The t coordinate.
		 */
		float t;

		/**
		 * @brief The p coordinate. (often referred to as r)
		 */
		float p;
	};
	struct
	{
		/**
		 * @brief The red component.
		 */
		float r;

		/**
		 * @brief The green component.
		 */
		float g;

		/**
		 * @brief The blue component.
		 */
		float b;
	};
} dsVector3f;

/**
 * @brief Structure for a 3D vector holding doubles.
 *
 * This can be accessed using cartesian coordinates (x, y, z), texture coordinates (s, t, p), color
 * channels (r, g, b), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 *
 * @remark When bracket initializing, use two brackets. (i.e. {{x, y, z}})
 * @see Vector3.h
 */
typedef union dsVector3d
{
	/**
	  * @brief Array of the vector values.
	  */
	double values[3];

	struct
	{
		/**
		 * @brief The x coordinate.
		 */
		double x;

		/**
		 * @brief The y coordinate.
		 */
		double y;

		/**
		 * @brief The z coordinate.
		 */
		double z;
	};
	struct
	{
		/**
		 * @brief The s coordinate.
		 */
		double s;

		/**
		 * @brief The t coordinate.
		 */
		double t;

		/**
		 * @brief The p coordinate. (often referred to as r)
		 */
		double p;
	};
	struct
	{
		/**
		 * @brief The red component.
		 */
		double r;

		/**
		 * @brief The green component.
		 */
		double g;

		/**
		 * @brief The blue component.
		 */
		double b;
	};
} dsVector3d;

/**
 * @brief Structure for a 3D vector holding ints.
 *
 * This can be accessed using cartesian coordinates (x, y, z), texture coordinates (s, t, p), color
 * channels (r, g, b), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 *
 * @remark When bracket initializing, use two brackets. (i.e. {{x, y, z}})
 * @see Vector3.h
 */
typedef union dsVector3i
{
	/**
	  * @brief Array of the vector values.
	  */
	int values[3];

	struct
	{
		/**
		 * @brief The x coordinate.
		 */
		int x;

		/**
		 * @brief The y coordinate.
		 */
		int y;

		/**
		 * @brief The z coordinate.
		 */
		int z;
	};
	struct
	{
		/**
		 * @brief The s coordinate.
		 */
		int s;

		/**
		 * @brief The t coordinate.
		 */
		int t;

		/**
		 * @brief The p coordinate. (often referred to as r)
		 */
		int p;
	};
	struct
	{
		/**
		 * @brief The red component.
		 */
		int r;

		/**
		 * @brief The green component.
		 */
		int g;

		/**
		 * @brief The blue component.
		 */
		int b;
	};
} dsVector3i;

/**
 * @brief Structure for a 3D vector holding 64-bit longs.
 *
 * This can be accessed using cartesian coordinates (x, y, z), texture coordinates (s, t, p), color
 * channels (r, g, b), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 *
 * @remark When bracket initializing, use two brackets. (i.e. {{x, y, z}})
 * @see Vector3.h
 */
typedef union dsVector3l
{
	/**
	  * @brief Array of the vector values.
	  */
	long long values[3];

	struct
	{
		/**
		 * @brief The x coordinate.
		 */
		long long x;

		/**
		 * @brief The y coordinate.
		 */
		long long y;

		/**
		 * @brief The z coordinate.
		 */
		long long z;
	};
	struct
	{
		/**
		 * @brief The s coordinate.
		 */
		long long s;

		/**
		 * @brief The t coordinate.
		 */
		long long t;

		/**
		 * @brief The p coordinate. (often referred to as r)
		 */
		long long p;
	};
	struct
	{
		/**
		 * @brief The red component.
		 */
		long long r;

		/**
		 * @brief The green component.
		 */
		long long g;

		/**
		 * @brief The blue component.
		 */
		long long b;
	};
} dsVector3l;

/**
 * @brief Structure for a 4D vector holding floats.
 *
 * This can be accessed using cartesian coordinates (x, y, z, w), texture coordinates (s, t, p, q),
 * colors channels (r, g, b, a), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 *
 * @remark When bracket initializing, use two brackets. (i.e. {{x, y, z, w}})
 * @see Vector4.h
 */
typedef union dsVector4f
{
	/**
	 * @brief Array of the vector values.
	 */
	float values[4];

	/**
	 * @brief SIMD value when supported.
	 */
#if DS_HAS_SIMD
	dsSIMD4f simd;
#endif

	struct
	{
		/**
		 * @brief The x coordinate.
		 */
		float x;

		/**
		 * @brief The y coordinate.
		 */
		float y;

		/**
		 * @brief The z coordinate.
		 */
		float z;

		/**
		 * @brief The w coordinate.
		 */
		float w;
	};
	struct
	{
		/**
		 * @brief The s coordinate.
		 */
		float s;

		/**
		 * @brief The t coordinate.
		 */
		float t;

		/**
		 * @brief The p coordinate. (often referred to as r)
		 */
		float p;

		/**
		 * @brief The q coordinate.
		 */
		float q;
	};
	struct
	{
		/**
		 * @brief The red component.
		 */
		float r;

		/**
		 * @brief The green component.
		 */
		float g;

		/**
		 * @brief The blue component.
		 */
		float b;

		/**
		 * @brief The alpha component.
		 */
		float a;
	};
} dsVector4f;

/**
 * @brief Structure for a 4D vector holding doubles.
 *
 * This can be accessed using cartesian coordinates (x, y, z, w), texture coordinates (s, t, p, q),
 * colors channels (r, g, b, a), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 *
 * @remark When bracket initializing, use two brackets. (i.e. {{x, y, z, w}})
 * @remark This will be 16 byte aligned when SIMD is supported. Usage with dsSIMD4d must be done
 *     manually, either using an unaligned load or manual alignment with DS_ALIGN() and allocation
 *     alignment.
 * @see Vector4.h
 */
typedef union dsVector4d
{
	/**
	 * @brief Array of the vector values.
	 */
	double values[4];

#if DS_HAS_SIMD
	/**
	 * @brief SIMD value as 2 doubles when supported.
	 */
	dsSIMD2d simd2[2];
#endif

	struct
	{
		/**
		 * @brief The x coordinate.
		 */
		double x;

		/**
		 * @brief The y coordinate.
		 */
		double y;

		/**
		 * @brief The z coordinate.
		 */
		double z;

		/**
		 * @brief The w coordinate.
		 */
		double w;
	};
	struct
	{
		/**
		 * @brief The s coordinate.
		 */
		double s;

		/**
		 * @brief The t coordinate.
		 */
		double t;

		/**
		 * @brief The p coordinate. (often referred to as r)
		 */
		double p;

		/**
		 * @brief The q coordinate.
		 */
		double q;
	};
	struct
	{
		/**
		 * @brief The red component.
		 */
		double r;

		/**
		 * @brief The green component.
		 */
		double g;

		/**
		 * @brief The blue component.
		 */
		double b;

		/**
		 * @brief The alpha component.
		 */
		double a;
	};
} dsVector4d;

/**
 * @brief Structure for a 4D vector holding ints.
 *
 * This can be accessed using cartesian coordinates (x, y, z, w), texture coordinates (s, t, p, q),
 * colors channels (r, g, b, a), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 *
 * @remark When bracket initializing, use two brackets. (i.e. {{x, y, z, w}})
 * @remark This will be 16 byte aligned when SIMD is supported to allow storing bool results.
 * @see Vector4.h
 */
typedef union dsVector4i
{
	/**
	 * @brief Array of the vector values.
	 */
	int values[4];

#if DS_HAS_SIMD
	/**
	 * @brief SIMD value when supported.
	 */
	dsSIMD4fb simd;
#endif

	struct
	{
		/**
		 * @brief The x coordinate.
		 */
		int x;

		/**
		 * @brief The y coordinate.
		 */
		int y;

		/**
		 * @brief The z coordinate.
		 */
		int z;

		/**
		 * @brief The w coordinate.
		 */
		int w;
	};
	struct
	{
		/**
		 * @brief The s coordinate.
		 */
		int s;

		/**
		 * @brief The t coordinate.
		 */
		int t;

		/**
		 * @brief The p coordinate. (often referred to as r)
		 */
		int p;

		/**
		 * @brief The q coordinate.
		 */
		int q;
	};
	struct
	{
		/**
		 * @brief The red component.
		 */
		int r;

		/**
		 * @brief The green component.
		 */
		int g;

		/**
		 * @brief The blue component.
		 */
		int b;

		/**
		 * @brief The alpha component.
		 */
		int a;
	};
} dsVector4i;

/**
 * @brief Structure for a 4D vector holding 64-bit longs.
 *
 * This can be accessed using cartesian coordinates (x, y, z, w), texture coordinates (s, t, p, q),
 * colors channels (r, g, b, a), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 *
 * @remark When bracket initializing, use two brackets. (i.e. {{x, y, z, w}})
 * @remark This will be 16 byte aligned when SIMD is supported. Usage with dsSIMD4db must be done
 *     manually, either using an unaligned load or manual alignment with DS_ALIGN() and allocation
 *     alignment.
 * @see Vector4.h
 */
typedef union dsVector4l
{
	/**
	 * @brief Array of the vector values.
	 */
	long long values[4];

#if DS_HAS_SIMD
	/**
	 * @brief SIMD value as 2 double bools when supported.
	 */
	dsSIMD2db simd2[2];
#endif

	struct
	{
		/**
		 * @brief The x coordinate.
		 */
		long long x;

		/**
		 * @brief The y coordinate.
		 */
		long long y;

		/**
		 * @brief The z coordinate.
		 */
		long long z;

		/**
		 * @brief The w coordinate.
		 */
		long long w;
	};
	struct
	{
		/**
		 * @brief The s coordinate.
		 */
		long long s;

		/**
		 * @brief The t coordinate.
		 */
		long long t;

		/**
		 * @brief The p coordinate. (often referred to as r)
		 */
		long long p;

		/**
		 * @brief The q coordinate.
		 */
		long long q;
	};
	struct
	{
		/**
		 * @brief The red component.
		 */
		long long r;

		/**
		 * @brief The green component.
		 */
		long long g;

		/**
		 * @brief The blue component.
		 */
		long long b;

		/**
		 * @brief The alpha component.
		 */
		long long a;
	};
} dsVector4l;

/**
 * @brief Structure for a color.
 *
 * This contains 8 byte color components for red, green, blue, and alpha.
 *
 * @remark When bracket initializing, use two brackets. (i.e. {{r, g, b, a}})
 */
typedef union dsColor
{
	/**
	 * @brief Array of the color values.
	 */
	uint8_t values[4];

	struct
	{
		/**
		 * @brief The red component.
		 */
		uint8_t r;

		/**
		 * @brief The green component.
		 */
		uint8_t g;

		/**
		 * @brief The blue component.
		 */
		uint8_t b;

		/**
		 * @brief The alpha component.
		 */
		uint8_t a;
	};
} dsColor;

/**
 * @brief Typedef for a 3-component floating point color.
 */
typedef union dsVector3f dsColor3f;

/**
 * @brief Typedef for a 4-component floating point color.
 */
typedef union dsVector4f dsColor4f;

/**
 * @brief structure for an HSV color.
 * @remark When bracket initializing, use two brackets. (i.e. {{h, s, v, a}})
 */
typedef union dsHSVColor
{
	/**
	 * @brief Array of the color values.
	 */
	float values[4];

	/**
	 * @brief SIMD value when supported.
	 */
#if DS_HAS_SIMD
	dsSIMD4f simd;
#endif

	struct
	{
		/**
		 * @brief The hue in the range [0, 360].
		 */
		float h;

		/**
		 * @brief The saturation in the range [0, 1].
		 */
		float s;

		/**
		 * @brief The value, usually in the range [0, 1].
		 */
		float v;

		/**
		 * @brief The alpha value in the range [0, 1].
		 */
		float a;
	};
} dsHSVColor;

/**
 * @brief structure for an HSL color.
 * @remark When bracket initializing, use two brackets. (i.e. {{h, s, l, a}})
 */
typedef union dsHSLColor
{
	/**
	 * @brief Array of the color values.
	 */
	float values[4];

	/**
	 * @brief SIMD value when supported.
	 */
#if DS_HAS_SIMD
	dsSIMD4f simd;
#endif

	struct
	{
		/**
		 * @brief The hue in the range [0, 360].
		 */
		float h;

		/**
		 * @brief The saturation in the range [0, 1].
		 */
		float s;

		/**
		 * @brief The lightness, usually in the range [0, 1].
		 */
		float l;

		/**
		 * @brief The alpha value in the range [0, 1].
		 */
		float a;
	};
} dsHSLColor;

/**
 * @brief Structure for a 2x2 matrix of floats.
 *
 * This can be accessed as an array of columns or a 2D array of values.
 *
 * @remark When bracket initializing, there must be two brackets around the list of column vectors.
 * (i.e. {{ {x0, y0}, {x1, y1} }})
 *
 * @see dsMatrix22.h
 */
typedef union dsMatrix22f
{
	/**
	 * @brief The values of the matrix.
	 */
	float values[2][2];

	/**
	 * @brief The columns of the matrix.
	 */
	dsVector2f columns[2];
} dsMatrix22f;

/**
 * @brief Structure for a 2x2 matrix of doubles.
 *
 * This can be accessed as an array of columns or a 2D array of values.
 *
 * @remark When bracket initializing, there must be two brackets around the list of column vectors.
 * (i.e. {{ {x0, y0}, {x1, y1} }})
 *
 * @see dsMatrix22.h
 */
typedef union dsMatrix22d
{
	/**
	 * @brief The values of the matrix.
	 */
	double values[2][2];

	/**
	 * @brief The columns of the matrix.
	 */
	dsVector2d columns[2];
} dsMatrix22d;

/**
 * @brief Structure for a 3x3 matrix of floats.
 *
 * This can be accessed as an array of columns or a 2D array of values.
 *
 * @remark When bracket initializing, there must be two brackets around the list of column vectors.
 * (i.e. {{ {x0, y0, z0}, {x1, y1, z1}, {x2, y2, z2} }})
 *
 * @see dsMatrix33.h
 */
typedef union dsMatrix33f
{
	/**
	 * @brief The values of the matrix.
	 */
	float values[3][3];

	/**
	 * @brief The columns of the matrix.
	 */
	dsVector3f columns[3];
} dsMatrix33f;

/**
 * @brief Structure for a 3x3 matrix of doubles.
 *
 * This can be accessed as an array of columns or a 2D array of values.
 *
 * @remark When bracket initializing, there must be two brackets around the list of column vectors.
 * (i.e. {{ {x0, y0, z0}, {x1, y1, z1}, {x2, y2, z2} }})
 *
 * @see dsMatrix33.h
 */
typedef union dsMatrix33d
{
	/**
	 * @brief The values of the matrix.
	 */
	double values[3][3];

	/**
	 * @brief The columns of the matrix.
	 */
	dsVector3d columns[3];
} dsMatrix33d;

/**
 * @brief Structure for a 4x4 matrix of floats.
 *
 * This can be accessed as an array of columns or a 2D array of values.
 *
 * @remark When bracket initializing, there must be two brackets around the list of column vectors.
 * (i.e. {{ {x0, y0, z0, w0}, {x1, y1, z1, w1}, {x2, y2, z2, w2}, {x3, y3, z3, w3} }})
 * @remark This will be 16 byte aligned when SIMD is supported.
 * @see dsMatrix44.h
 */
typedef union dsMatrix44f
{
	/**
	 * @brief The values of the matrix.
	 */
	float values[4][4];

	/**
	 * @brief The columns of the matrix.
	 */
	dsVector4f columns[4];
} dsMatrix44f;

/**
 * @brief Structure for a 4x4 matrix of doubles.
 *
 * This can be accessed as an array of columns or a 2D array of values.
 *
 * @remark When bracket initializing, there must be two brackets around the list of column vectors.
 * (i.e. {{ {x0, y0, z0, w0}, {x1, y1, z1, w1}, {x2, y2, z2, w2}, {x3, y3, z3, w3} }})
 * @see dsMatrix44.h
 */
typedef union dsMatrix44d
{
	/**
	 * @brief The values of the matrix.
	 */
	double values[4][4];

	/**
	 * @brief The columns of the matrix.
	 */
	dsVector4d columns[4];
} dsMatrix44d;

/**
 * @brief Structure that holds a half float.
 * @see Packing.h
 */
typedef struct dsHalfFloat
{
	/**
	 * @brief The data for the half float.
	 */
	uint16_t data;
} dsHalfFloat;

/**
 * @brief Structure that holds a quaternion, typically used for rotation, as floats.
 * @see Quaternion.h
 */
typedef union dsQuaternion4f
{
	/**
	 * @brief The values fo the quaternion.
	 */
	float values[4];

	/**
	 * @brief SIMD value when supported.
	 */
#if DS_HAS_SIMD
	dsSIMD4f simd;
#endif

	struct
	{
		/**
		 * @brief The i coordinate.
		 */
		float i;

		/**
		 * @brief The j coordinate.
		 */
		float j;

		/**
		 * @brief The k coordinate.
		 */
		float k;

		/**
		 * @brief The real coordinate.
		 */
		float r;
	};
} dsQuaternion4f;

/**
 * @brief Structure that holds a quaternion, typically used for rotation, as doubles.
 * @see Quaternion.h
 */
typedef union dsQuaternion4d
{
	/**
	 * @brief The values fo the quaternion.
	 */
	double values[4];

	struct
	{
		/**
		 * @brief The i coordinate.
		 */
		double i;

		/**
		 * @brief The j coordinate.
		 */
		double j;

		/**
		 * @brief The k coordinate.
		 */
		double k;

		/**
		 * @brief The real coordinate.
		 */
		double r;
	};
} dsQuaternion4d;

/**
 * @brief Structure that holds the state for random number generation.
 * @remark This must not be zero-initialized! Use dsRandom_seed() to initialize the state properly,
 *     even if you use a default seed of 0.
 * @see Random.h
 */
typedef struct dsRandom
{
	/**
	 * @brief The state of the random number generator.
	 *
	 * This should not be modified directly outside of the implementation unless you wish to save
	 * and restore the state. It should never be zero-initialized.
	 */
	uint64_t state[4];
} dsRandom;

#ifdef __cplusplus
}
#endif

// Needs to be after the extern "C" block.
/// @cond
DS_ENUM_BITMASK_OPERATORS(dsProjectionMatrixOptions);
/// @endcond
