/*
 * Copyright 2017 Aaron Barany
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
#include <DeepSea/Core/Memory/Types.h>
#include <DeepSea/Math/Types.h>
#include <DeepSea/Render/Types.h>
#include <DeepSea/Text/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/VectorDraw library.
 */

/**
 * @brief Log tag used by the vector draw library.
 */
#define DS_VECTOR_DRAW_LOG_TAG "vector"

/**
 * @brief Constant for no clipping path to be applied.
 */
#define DS_VECTOR_NO_CLIP_PATH (uint32_t)-1

/**
 * @brief Constant for the maximum length of a vector resource name, including the null terminator.
 */
#define DS_MAX_VECTOR_RESOURCE_NAME_LENGTH 100U

/**
 * @brief Constant for the maximum number of materials allowed for a single material set.
 */
#define DS_MAX_ALLOWED_VECTOR_MATERIALS 1024U

/**
 * @brief Enum for a command for vector drawing.
 */
typedef enum dsVectorCommandType
{
	dsVectorCommandType_StartPath,  ///< Starts a new path. The startPath field will be set.
	dsVectorCommandType_Move,       ///< Moves the cursor to a position. The move field will be set.
	dsVectorCommandType_Line,       ///< Draws a line. The line field will be set.
	dsVectorCommandType_Bezier,     ///< Draws a Bezier curve. The bezier field will be set.
	dsVectorCommandType_Quadratic,  ///< Draws a quadratic curve. The quadratic field will be set.
	dsVectorCommandType_Arc,        ///< Draws an arc. The arc field will be set.
	dsVectorCommandType_ClosePath,  ///< Closes the current path. No field will be set.
	dsVectorCommandType_Ellipse,    ///< Draws an ellipse. The ellipse field will be set.
	dsVectorCommandType_Rectangle,  ///< Draws a rectangle. The rectangle field will be set.
	dsVectorCommandType_StrokePath, ///< Strokes the current path. The strokePath field will be set.
	dsVectorCommandType_FillPath,   ///< Fills the current path. The fillPath field will be set.
	dsVectorCommandType_TextPath,   ///< Draws text along a path. The textPath field will be set.
	dsVectorCommandType_Text,       ///< Draws text. The text field will be set.
	dsVectorCommandType_TextRange,  ///< Gives a range of information for text. The textRange field
	                                ///< will be set.
	dsVectorCommandType_Image       ///< Draws an image. The image field will be set.
} dsVectorCommandType;

/**
 * @brief Enum for the type of material.
 */
typedef enum dsVectorMaterialType
{
	dsVectorMaterialType_Color,          ///< Solid color. The color field will be set.
	dsVectorMaterialType_LinearGradient, ///< Linear gradient. The linearGradient field will be set.
	dsVectorMaterialType_RadialGradient  ///< Radial gradient. The radialGradient field will be set.
} dsVectorMaterialType;

/**
 * @brief Enum for the coordinate space of a vector element.
 */
typedef enum dsVectorElementSpace
{
	dsVectorSpace_Local, ///< Local coordinate space of the object.
	dsVectorSpace_Bounds ///< Space determined by the bounding box of the element.
} dsVectorElementSpace;

/**
 * @brief Enum for what to do at the edge of a gradient.
 */
typedef enum dsGradientEdge
{
	dsGradientEdge_Clamp,  ///< Clamp at the edge of the gradient.
	dsGradientEdge_Repeat, ///< Repeat at the edge of the gradient.
	dsGradientEdge_Mirror  ///< Repeat in the reverse direction at the edge of the gradient.
} dsGradientEdge;

/**
 * @brief Enum for the join type of a line.
 */
typedef enum dsLineJoin
{
	dsLineJoin_Miter, ///< The line is joined with a sharp corner.
	dsLineJoin_Bevel, ///< Similar to miter, but the corner is rounded.
	dsLineJoin_Round  ///< The line is joined with a rounded cap.
} dsLineJoin;

/**
 * @brief Enum for the cap type of a line.
 */
typedef enum dsLineCap
{
	dsLineCap_Butt,  ///< The line ends immediately at the end point.
	dsLineCap_Round, ///< The line ends with a round cap.
	dsLineCap_Square ///< The line ends with a square cap.
} dsLineCap;

/**
 * @brief Enum for how to interpret the position for text.
 */
typedef enum dsVectorTextPosition
{
	dsVectorTextPosition_None,    ///< Don't use the position element.
	dsVectorTextPosition_Offset,  ///< Offset from the current position.
	dsVectorTextPosition_Absolute ///< Absolute position.
} dsVectorTextPosition;

/**
 * @brief Struct containing a stop for a gradient.
 * @see Gradient.h
 */
typedef struct dsGradientStop
{
	/**
	 * @brief The position of the gradient in the range [0, 1].
	 */
	float position;

	/**
	 * @brief The color of the stop.
	 */
	dsColor color;
} dsGradientStop;

/**
 * @brief Struct containing information about a gradient.
 * @see Gradient.h
 */
typedef struct dsGradient
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The stops for the gradient.
	 */
	dsGradientStop* stops;

	/**
	 * @brief The number of stops in the gradient.
	 */
	uint32_t stopCount;
} dsGradient;

/**
 * @brief Struct containing information about a linear gradient.
 */
typedef struct dsLinearGradient
{
	/**
	 * @brief The base gradient.
	 */
	const dsGradient* gradient;

	/**
	 * @brief The start position of the gradient.
	 */
	dsVector2f start;

	/**
	 * @brief The end position of the gradient.
	 */
	dsVector2f end;

	/**
	 * @brief What to do at the edge of the gradient.
	 */
	dsGradientEdge edge;

	/**
	 * @brief The coordinate space of the gradient.
	 */
	dsVectorElementSpace coordinateSpace;

	/**
	 * @brief The transform for the gradient.
	 */
	dsMatrix33f transform;
} dsLinearGradient;

/**
 * @brief Struct containing information about a radial gradient.
 */
typedef struct dsRadialGradient
{
	/**
	 * @brief The base gradient.
	 */
	const dsGradient* gradient;

	/**
	 * @brief The center of the gradient.
	 */
	dsVector2f center;

	/**
	 * @brief The radius of the circle for the gradient.
	 */
	float radius;

	/**
	 * @brief The focus point of the gradient.
	 *
	 * This can be set to offset the gradeint from the center.
	 */
	dsVector2f focus;

	/**
	 * @brief The radius of the focus.
	 */
	float focusRadius;

	/**
	 * @brief What to do at the edge of the gradient.
	 */
	dsGradientEdge edge;

	/**
	 * @brief The coordinate space of the gradient.
	 */
	dsVectorElementSpace coordinateSpace;

	/**
	 * @brief The transform for the gradient.
	 */
	dsMatrix33f transform;
} dsRadialGradient;

/**
 * @brief Struct containing information about the material for a vector element.
 * @see VectorMaterial.h
 */
typedef struct dsVectorMaterial
{
	/**
	 * @brief The type of the material.
	 */
	dsVectorMaterialType materialType;

	union
	{
		/**
		 * @brief The color of the material.
		 *
		 * This will be set when type is dsVectorMaterialType_Color.
		 */
		dsColor color;

		/**
		 * @brief The linear gradient of the material.
		 *
		 * This will be set when type is dsVectorMaterialType_LinearGradient.
		 */
		dsLinearGradient linearGradient;

		/**
		 * @brief The radial gradient of the material.
		 *
		 * This will be set when type is dsVectorMaterialType_RadialGradient.
		 */
		dsRadialGradient radialGradient;
	};
} dsVectorMaterial;

/**
 * @brief Struct containing information for starting a path.
 */
typedef struct dsVectorCommandStartPath
{
	/**
	 * @brief The transform for the path.
	 */
	dsMatrix33f transform;
} dsVectorCommandStartPath;

/**
 * @brief Struct containing information for a move command.
 */
typedef struct dsVectorCommandMove
{
	/**
	 * @brief The position to move to.
	 */
	dsVector2f position;
} dsVectorCommandMove;

/**
 * @brief Struct containing information for a line command.
 */
typedef struct dsVectorCommandLine
{
	/**
	 * @brief The position to draw the line to.
	 */
	dsVector2f end;
} dsVectorCommandLine;

/**
 * @brief Struct containing information for a bezier command.
 */
typedef struct dsVectorCommandBezier
{
	/**
	 * @brief The first control point.
	 */
	dsVector2f control1;

	/**
	 * @brief The second control point.
	 */
	dsVector2f control2;

	/**
	 * @brief The end point of the curve.
	 */
	dsVector2f end;
} dsVectorCommandBezier;

/**
 * @brief Struct containing information for a quadratic command.
 */
typedef struct dsVectorCommandQuadratic
{
	/**
	 * @brief The control point.
	 */
	dsVector2f control;

	/**
	 * @brief The end point of the curve.
	 */
	dsVector2f end;
} dsVectorCommandQuadratic;

/**
 * @brief Struct containing information for an arc command.
 */
typedef struct dsVectorCommandArc
{
	/**
	 * @brief The radius of the arc on the X and Y axes.
	 */
	dsVector2f radius;

	/**
	 * @brief The rotation to apply to the axes in radians.
	 */
	float rotation;

	/**
	 * @brief True for a large arc, which will choose a path to take > 180 degrees.
	 */
	bool largeArc;

	/**
	 * @brief True to choose an ellipse that follows a clockwise path around the center.
	 *
	 * This can control if the arc path will be convex or concave. This is also known as the sweep
	 * parameter.
	 */
	bool clockwise;

	/**
	 * @brief The end position of the arc.
	 */
	dsVector2f end;
} dsVectorCommandArc;

/**
 * @brief Struct containing information for an ellipse command.
 */
typedef struct dsVectorCommandEllipse
{
	/**
	 * @brief The center of the ellipse.
	 */
	dsVector2f center;

	/**
	 * @brief The radius of the ellipse.
	 */
	dsVector2f radius;
} dsVectorCommandEllipse;

/**
 * @brief Struct containing information for a rectangle command.
 */
typedef struct dsVectorCommandRectangle
{
	/**
	 * @brief The bounds of the rectangle.
	 */
	dsAlignedBox2f bounds;

	/**
	 * @brief The radius of the corners.
	 */
	dsVector2f cornerRadius;
} dsVectorCommandRectangle;

/**
 * @brief Struct containing information for a stroke path command.
 */
typedef struct dsVectorCommandStrokePath
{
	/**
	 * @brief The material to apply.
	 */
	const char* material;

	/**
	 * @brief The opacity of the stroke.
	 */
	float opacity;

	/**
	 * @brief The join type for the line.
	 */
	dsLineJoin joinType;

	/**
	 * @brief The cap type for the line.
	 */
	dsLineCap capType;

	/**
	 * @brief The width of the stroke.
	 */
	float width;

	/**
	 * @brief The miter limit of the stroke.
	 */
	float miterLimit;

	/**
	 * @brief Dash and gap distances, respectively, for up to two dash patterns.
	 */
	dsVector4f dashArray;
} dsVectorCommandStrokePath;

/**
 * @brief Struct containing information for a fill path command.
 */
typedef struct dsVectorCommandFillPath
{
	/**
	 * @brief The material to apply.
	 */
	const char* material;

	/**
	 * @brief The opacity of the fill.
	 */
	float opacity;
} dsVectorCommandFillPath;

/**
 * @brief Struct containing information for a text path command.
 */
typedef struct dsVectorCommandTextPath
{
	/**
	 * @brief The string to display.
	 */
	const void* string;

	/**
	 * @brief The encoding type of the string.
	 */
	dsUnicodeType stringType;

	/**
	 * @brief The font to use with the string.
	 */
	dsFont* font;

	/**
	 * @brief The number of range commands that will follow this.
	 */
	uint32_t rangeCount;
} dsVectorCommandTextPath;

/**
 * @brief Struct containing information for a text command.
 */
typedef struct dsVectorCommandText
{
	/**
	 * @brief The string to display.
	 */
	const void* string;

	/**
	 * @brief The encoding type of the string.
	 */
	dsUnicodeType stringType;

	/**
	 * @brief The font to use with the string.
	 */
	dsFont* font;

	/**
	 * @brief The justification of the text.
	 */
	dsTextJustification justification;

	/**
	 * @brief The transform matrix for the text.
	 */
	dsMatrix33f transform;

	/**
	 * @brief The number of range commands that will follow this.
	 */
	uint32_t rangeCount;
} dsVectorCommandText;

/**
 * @brief Struct containing information for a range of text.
 */
typedef struct dsVectorCommandTextRange
{
	/**
	 * @brief The first codepoint of the range.
	 */
	uint32_t start;

	/**
	 * @brief The number of codepoints in the range.
	 */
	uint32_t count;

	/**
	 * @brief How to interpret the position.
	 */
	dsVectorTextPosition positionType;

	/**
	 * @brief The position of the text.
	 */
	dsVector2f position;

	/**
	 * @brief The material for the text fill, or NULL if no fill.
	 */
	const char* fillMaterial;

	/**
	 * @brief The material for the text outline, or NULL if no outline.
	 */
	const char* outlineMaterial;

	/**
	 * @brief The opacity of the fill.
	 */
	float fillOpacity;

	/**
	 * @brief The opacity of the outline.
	 */
	float outlineOpacity;

	/**
	 * @brief The size of the text.
	 */
	float size;

	/**
	 * @brief The amount to embolden the text.
	 */
	float embolden;

	/**
	 * @brief The amount to slant the text.
	 */
	float slant;

	/**
	 * @brief The width of the outline.
	 */
	float outlineWidth;

	/**
	 * @brief The amount to anti-alias the text.
	 */
	float antiAlias;
} dsVectorCommandTextRange;

/**
 * @brief Struct containing information for an image command.
 */
typedef struct dsVectorCommandImage
{
	/**
	 * @brief The image to draw.
	 */
	dsTexture* image;

	/**
	 * @brief The bounds in which to display the image.
	 */
	dsAlignedBox2f imageBounds;

	/**
	 * @brief The opacity of the image.
	 */
	float opacity;

	/**
	 * @brief The transform matrix for the image.
	 */
	dsMatrix33f transform;
} dsVectorCommandImage;

/**
 * @brief Struct containing a command for vector rendering.
 */
typedef struct dsVectorCommand
{
	/**
	 * @brief The type of the command.
	 */
	dsVectorCommandType commandType;

	union
	{
		/**
		 * @brief Start path command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_StartPath.
		 */
		dsVectorCommandStartPath startPath;

		/**
		 * @brief Move command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_Move.
		 */
		dsVectorCommandMove move;

		/**
		 * @brief Line command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_Line.
		 */
		dsVectorCommandLine line;

		/**
		 * @brief Bezier command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_Bezier.
		 */
		dsVectorCommandBezier bezier;

		/**
		 * @brief Quadratic command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_Quadratic.
		 */
		dsVectorCommandQuadratic quadratic;

		/**
		 * @brief Arc command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_Arc.
		 */
		dsVectorCommandArc arc;

		/**
		 * @brief Stroke path command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_StrokePath.
		 */
		dsVectorCommandStrokePath strokePath;

		/**
		 * @brief Fill path command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_FillPath.
		 */
		dsVectorCommandFillPath fillPath;

		/**
		 * @brief Text path command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_TextPath.
		 */
		dsVectorCommandTextPath textPath;

		/**
		 * @brief Text command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_Text.
		 */
		dsVectorCommandText text;

		/**
		 * @brief Text range command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_TextRange.
		 */
		dsVectorCommandTextRange textRange;

		/**
		 * @brief Image command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_Image.
		 */
		dsVectorCommandImage image;

		/**
		 * @brief Ellipse command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_Ellipse.
		 */
		dsVectorCommandEllipse ellipse;

		/**
		 * @brief Rectangle command parameters.
		 *
		 * This will be set if commandType is dsVectorCommandType_Rectangle.
		 */
		dsVectorCommandRectangle rectangle;
	};
} dsVectorCommand;

/**
 * @brief Struct containing a resizeable buffer of commands.
 * @see VectorCommandBuffer.h
 */
typedef struct dsVectorCommandBuffer
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The list of commands.
	 */
	dsVectorCommand* commands;

	/**
	 * @brief The number of currently active commands.
	 */
	uint32_t commandCount;

	/**
	 * @brief The current maximum number of commands before a resize is required.
	 */
	uint32_t maxCommands;
} dsVectorCommandBuffer;

/**
 * @brief Struct containing the shader module used by vector images.
 * @see VectorShaderModule.h
 */
typedef struct dsVectorShadersModule
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The module containing the shaders.
	 */
	dsShaderModule* shaderModule;

	/**
	 * @brief The transfor shader variable group description.
	 */
	dsShaderVariableGroupDesc* transformDesc;

	/**
	 * @brief The material description.
	 */
	dsMaterialDesc* materialDesc;

	/**
	 * @brief The element index for the image-space transform matrix.
	 */
	uint32_t transformElement;

	/**
	 * @brief The element index for the model-view-projection matrix.
	 */
	uint32_t modelViewProjectionElement;

	/**
	 * @brief The element index for the image size.
	 */
	uint32_t sizeElement;

	/**
	 * @brief The element index for the mateial and vector info texture sizes.
	 */
	uint32_t textureSizesElement;

	/**
	 * @brief The volatile material ID for the transform shader variables.
	 */
	uint32_t transformId;

	/**
	 * @brief The volatile material ID for the shape info texture.
	 */
	uint32_t shapeInfoTextureId;

	/**
	 * @brief The volatile material ID for the material info texture.
	 */
	uint32_t materialInfoTextureId;

	/**
	 * @brief The volatile material ID for the material texture.
	 */
	uint32_t materialColorTextureId;

	/**
	 * @brief The volatile material ID for the font texture.
	 */
	uint32_t fontTextureId;

	/**
	 * @brief The index for the vector shape shader.
	 */
	uint32_t shapeShaderIndex;

	/**
	 * @brief The index for the vector image shader.
	 */
	uint32_t imageShaderIndex;

	/**
	 * @brief The index for the vector text shader.
	 */
	uint32_t textShaderIndex;
} dsVectorShaderModule;

/**
 * @brief Struct that contains the context information for drawing vector images.
 * @see VectorShaderModule.h
 */
typedef dsShaderVariableGroup dsVectorDrawContext;

/**
 * @brief Struct containing the shaders used by vector images.
 * @see VectorShaders.h
 */
typedef struct dsVectorShaders
{
	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The vector shader module.
	 */
	dsVectorShaderModule* shaderModule;

	/**
	 * @brief The shader for the shape portions.
	 */
	dsShader* shapeShader;

	/**
	 * @brief The shader for the bitmap image portions.
	 */
	dsShader* imageShader;

	/**
	 * @brief The shader for the text portions.
	 */
	dsShader* textShader;
} dsVectorShaders;

/**
 * @brief Struct containing shared resources for vector graphics.
 * @see VectorResources.h
 */
typedef struct dsVectorResources dsVectorResources;

/**
 * @brief Struct containing a set of materials to be used in a vector image.
 * @see VectorMaterialSet.h
 */
typedef struct dsVectorMaterialSet dsVectorMaterialSet;

/**
 * @brief Struct containing temporary data when building up a vector image.
 * @see VectorScratchData.h
 */
typedef struct dsVectorScratchData dsVectorScratchData;

/**
 * @brief Struct containing information for a vector image that can be drawn.
 * @see VectorImage.h
 */
typedef struct dsVectorImage dsVectorImage;

#ifdef __cplusplus
}
#endif
