/*
 * Copyright 2016 Aaron Barany
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
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/Math library.
 */

/**
 * @brief Structure for a 2D vector holding floats.
 *
 * This can be accessed using cartesian coordinates (x, y), texture coordinates (s, t), color
 * channels (r, g), or an array of values.
 */
typedef struct dsVector2f
{
	union
	{
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

		/**
		 * @brief Array of the vector values.
		 */
		float values[2];
	};
} dsVector2f;

/**
 * @brief Structure for a 2D vector holding doubles.
 *
 * This can be accessed using cartesian coordinates (x, y), texture coordinates (s, t), color
 * channels (r, g), or an array of values.
 */
typedef struct dsVector2d
{
	union
	{
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

		/**
		 * @brief Array of the vector values.
		 */
		double values[2];
	};
} dsVector2d;

/**
 * @brief Structure for a 2D vector holding ints.
 *
 * This can be accessed using cartesian coordinates (x, y), texture coordinates (s, t), color
 * channels (r, g), or an array of values.
 */
typedef struct dsVector2i
{
	union
	{
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

		/**
		 * @brief Array of the vector values.
		 */
		int values[2];
	};
} dsVector2i;

/**
 * @brief Structure for a 3D vector holding floats.
 *
 * This can be accessed using cartesian coordinates (x, y, z), texture coordinates (s, t, p), color
 * channels (r, g, b), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 */
typedef struct dsVector3f
{
	union
	{
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

		/**
		 * @brief Array of the vector values.
		 */
		float values[3];
	};
} dsVector3f;

/**
 * @brief Structure for a 3D vector holding doubles.
 *
 * This can be accessed using cartesian coordinates (x, y, z), texture coordinates (s, t, p), color
 * channels (r, g, b), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 */
typedef struct dsVector3d
{
	union
	{
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

		/**
		 * @brief Array of the vector values.
		 */
		double values[3];
	};
} dsVector3d;

/**
 * @brief Structure for a 3D vector holding ints.
 *
 * This can be accessed using cartesian coordinates (x, y, z), texture coordinates (s, t, p), color
 * channels (r, g, b), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 */
typedef struct dsVector3i
{
	union
	{
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

		/**
		 * @brief Array of the vector values.
		 */
		int values[3];
	};
} dsVector3i;

/**
 * @brief Structure for a 4D vector holding floats.
 *
 * This can be accessed using cartesian coordinates (x, y, z, w), texture coordinates (s, t, p, q),
 * colors channels (r, g, b, a), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 *
 * This is guaranteed to be 16-byte aligned so it can be easily loaded into SIMD types.
 */
typedef struct dsVector4f
{
	union
	{
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

		/**
		 * @brief Array of the vector values.
		 */
		float values[4];
	};
} DS_ALIGN(16) dsVector4f;

/**
 * @brief Structure for a 4D vector holding doubles.
 *
 * This can be accessed using cartesian coordinates (x, y, z, w), texture coordinates (s, t, p, q),
 * colors channels (r, g, b, a), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 */
typedef struct dsVector4d
{
	union
	{
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

		/**
		 * @brief Array of the vector values.
		 */
		double values[4];
	};
} dsVector4d;

/**
 * @brief Structure for a 4D vector holding ints.
 *
 * This can be accessed using cartesian coordinates (x, y, z, w), texture coordinates (s, t, p, q),
 * colors channels (r, g, b, a), or an array of values.
 *
 * Note that p is used in place of r for texture coordinates to avoid naming conflicts.
 */
typedef struct dsVector4i
{
	union
	{
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

		/**
		 * @brief Array of the vector values.
		 */
		int values[4];
	};
} dsVector4i;

/**
 * @brief Structure for a color.
 *
 * This contains 8 byte color components for red, green, blue, and alpha..
 */
typedef struct dsColor
{
	union
	{
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

		/**
		 * @brief Array of the color values.
		 */
		uint8_t values[4];
	};
} dsColor;

/**
 * @brief Typedef for a 3-component floating point color.
 */
typedef struct dsVector3f dsColor3f;

/**
 * @brief Typedef for a 4-component floating point color.
 */
typedef struct dsVector4f dsColor4f;

/**
 * @brief Structure for a 2x2 matrix of floats.
 *
 * This can be accessed as an array of columns or a 2D array of values.
 */
typedef struct dsMatrix22f
{
	union
	{
		/**
		 * @brief The columns of the matrix.
		 */
		dsVector2f columns[2];

		/**
		 * @brief The values of the matrix.
		 */
		float values[2][2];
	};
} dsMatrix22f;

/**
 * @brief Structure for a 2x2 matrix of doubles.
 *
 * This can be accessed as an array of columns or a 2D array of values.
 */
typedef struct dsMatrix22d
{
	union
	{
		/**
		 * @brief The columns of the matrix.
		 */
		dsVector2d columns[2];

		/**
		 * @brief The values of the matrix.
		 */
		double values[2][2];
	};
} dsMatrix22d;

/**
 * @brief Structure for a 3x3 matrix of floats.
 *
 * This can be accessed as an array of columns or a 2D array of values.
 */
typedef struct dsMatrix33f
{
	union
	{
		/**
		 * @brief The columns of the matrix.
		 */
		dsVector3f columns[3];

		/**
		 * @brief The values of the matrix.
		 */
		float values[3][3];
	};
} dsMatrix33f;

/**
 * @brief Structure for a 3x3 matrix of doubles.
 *
 * This can be accessed as an array of columns or a 2D array of values.
 */
typedef struct dsMatrix33d
{
	union
	{
		/**
		 * @brief The columns of the matrix.
		 */
		dsVector3d columns[3];

		/**
		 * @brief The values of the matrix.
		 */
		double values[3][3];
	};
} dsMatrix33d;

/**
 * @brief Structure for a 4x4 matrix of floats.
 *
 * This can be accessed as an array of columns or a 2D array of values.
 */
typedef struct dsMatrix44f
{
	union
	{
		/**
		 * @brief The columns of the matrix.
		 */
		dsVector4f columns[4];

		/**
		 * @brief The values of the matrix.
		 */
		float values[4][4];
	};
} dsMatrix44f;

/**
 * @brief Structure for a 4x4 matrix of doubles.
 *
 * This can be accessed as an array of columns or a 2D array of values.
 */
typedef struct dsMatrix44d
{
	union
	{
		/**
		 * @brief The columns of the matrix.
		 */
		dsVector4d columns[4];

		/**
		 * @brief The values of the matrix.
		 */
		double values[4][4];
	};
} dsMatrix44d;

/**
 * @brief Structure that holds a half float.
 */
typedef struct dsHalfFloat
{
	/**
	 * @brief The data for the half float.
	 */
	uint16_t data;
} dsHalfFloat;

#ifdef __cplusplus
}
#endif
