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
#include <DeepSea/VectorDraw/Export.h>
#include <DeepSea/VectorDraw/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating a buffer of vector commands.
 *
 * This can be used to dynamically build up a command buffer.
 *
 * @see dsVectorCommandBuffer
 */

/**
 * @brief Initializes a command buffer.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to initialize.
 * @param allocator The allocator to use.
 * @param capacity The initial capacity. This may be 0.
 * @return False if an error occurred.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_initialize(dsVectorCommandBuffer* commandBuffer,
	dsAllocator* allocator, uint32_t capacity);

/**
 * @brief Adds a start path command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param transform The transform, or NULL for identity.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addStartPath(dsVectorCommandBuffer* commandBuffer,
	const dsMatrix33f* transform);

/**
 * @brief Adds a move command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param position The position to move to.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addMove(dsVectorCommandBuffer* commandBuffer,
	const dsVector2f* position);

/**
 * @brief Adds a line command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param end The end position of the line.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addLine(dsVectorCommandBuffer* commandBuffer,
	const dsVector2f* end);

/**
 * @brief Adds a bezier command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param control1 The first control point.
 * @param control2 The second control point.
 * @param end The end position of the bezier.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addBezier(dsVectorCommandBuffer* commandBuffer,
	const dsVector2f* control1, const dsVector2f* control2, const dsVector2f* end);

/**
 * @brief Adds a quadratic command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param control The control point.
 * @param end The end position of the bezier.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addQuadratic(dsVectorCommandBuffer* commandBuffer,
	const dsVector2f* control, const dsVector2f* end);

/**
 * @brief Adds an arc command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param radius The X and Y radius for the arc.
 * @param rotation The rotation of the arc in radians.
 * @param largeArc True to take a path that's > 180 degrees.
 * @param clockwise True to choose an ellipse that follows a clockwise path around the center. This
 *     can control if the arc path will be convex or concave. This is also known as the sweep
 *     parameter.
 * @param end The end position of the arc.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addArc(dsVectorCommandBuffer* commandBuffer,
	const dsVector2f* radius, float rotation, bool largeArc, bool clockwise, const dsVector2f* end);

/**
 * @brief Adds an ellipse command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param center The center of the ellipse.
 * @param radius The radius of the ellipse.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addEllipse(dsVectorCommandBuffer* commandBuffer,
	const dsVector2f* center, const dsVector2f* radius);

/**
 * @brief Adds a rectangle command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param bounds The bounds of the rectangle.
 * @param cornerRadius The radii of the corners in the X and Y direction, or NULL if no rounding.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addRectangle(dsVectorCommandBuffer* commandBuffer,
	const dsAlignedBox2f* bounds, const dsVector2f* cornerRadius);

/**
 * @brief Adds a stroke path command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param material The material to apply.
 * @param opacity The opacity of the stroke.
 * @param joinType The join type of the lines.
 * @param capType The cap type of the lines.
 * @param width The width of the stroke.
 * @param miterLimit The miter limit of the stroke.
 * @param dashArray Dash and gap distances, respectively, for up to two dash patterns. NULL is the
 *     same as providing all 0s, indicating no dashes.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addStrokePath(dsVectorCommandBuffer* commandBuffer,
	const char* material, float opacity, dsLineJoin joinType, dsLineCap capType, float width,
	float miterLimit, const dsVector4f* dashArray);

/**
 * @brief Adds a fill path command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param material The material to apply.
 * @param opacity The opacity of the fill.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addFillPath(dsVectorCommandBuffer* commandBuffer,
	const char* material, float opacity);

/**
 * @brief Adds a text path command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param string The string to display.
 * @param stringType The encoding type of the string.
 * @param font The font to use with the string.
 * @param rangeCount The number of range commands that will follow this.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addTextPath(dsVectorCommandBuffer* commandBuffer,
	const void* string, dsUnicodeType stringType, dsFont* font, uint32_t rangeCount);

/**
 * @brief Adds a text command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param string The string to display.
 * @param stringType The encoding type of the string.
 * @param font The font to use with the string.
 * @param justification The justification of the text.
 * @param transform The transform, or NULL for identity.
 * @param rangeCount The number of range commands that will follow this.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addText(dsVectorCommandBuffer* commandBuffer,
	const void* string, dsUnicodeType stringType, dsFont* font, dsTextJustification justification,
	const dsMatrix33f* transform, uint32_t rangeCount);

/**
 * @brief Adds a text range command to provide information for a previous text or text path command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param start The first codepoint of the range. Use the unicode functions in Text/Unicode.h to
 *     find codepoints within the string.
 * @param count The number of codepoints in the range.
 * @param positionType How to interpret the position.
 * @param position The position of the text. This may be NULL only if positionType is
 *     dsVectorTextPosition_None.
 * @param fillMaterial The material for the text fill, or NULL if no fill.
 * @param outlineMaterial The material for the text outline, or NULL if no outline.
 * @param fillOpacity The opacity of the fill material.
 * @param outlineOpacity The opacity of the outline material.
 * @param size The size of the text.
 * @param embolden The amount to embolden the text.
 * @param slant The amount to slant the text.
 * @param outlineWidth The width of the outline.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addTextRange(dsVectorCommandBuffer* commandBuffer,
	uint32_t start, uint32_t count, dsVectorTextPosition positionType, const dsVector2f* position,
	const char* fillMaterial, const char* outlineMaterial, float fillOpacity, float outlineOpacity,
	float size, float embolden, float slant, float outlineWidth);

/**
 * @brief Adds an image command.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to add the command to.
 * @param image The image to draw.
 * @param imageBounds The bounds in which to display the image.
 * @param opacity The opacity of the image.
 * @param transform The transform, or NULL for identity.
 * @return False if the command couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorCommandBuffer_addImage(dsVectorCommandBuffer* commandBuffer,
	dsTexture* image, const dsAlignedBox2f* imageBounds, float opacity,
	const dsMatrix33f* transform);

/**
 * @brief Destroys the memory stored within a command buffer.
 * @param commandBuffer The command buffer to destroy.
 */
DS_VECTORDRAW_EXPORT void dsVectorCommandBuffer_shutdown(dsVectorCommandBuffer* commandBuffer);

#ifdef __cplusplus
}
#endif
