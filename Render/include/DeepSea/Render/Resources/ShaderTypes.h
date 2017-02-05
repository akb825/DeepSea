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
#include <DeepSea/Core/Memory/Types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Types that are used for the shader and material system.
 */

/**
 * @brief Enum for the type of a material member.
 */
typedef enum dsMaterialType
{
	// Scalars and vectors
	dsMaterialType_Float,  ///< float
	dsMaterialType_Vec2,   ///< vec2
	dsMaterialType_Vec3,   ///< vec3
	dsMaterialType_Vec4,   ///< vec4
	dsMaterialType_Double, ///< double
	dsMaterialType_DVec2,  ///< dvec2
	dsMaterialType_DVec3,  ///< dvec3
	dsMaterialType_DVec4,  ///< dvec4
	dsMaterialType_Int,    ///< int
	dsMaterialType_IVec2,  ///< ivec2
	dsMaterialType_IVec3,  ///< ivec3
	dsMaterialType_IVec4,  ///< ivec4
	dsMaterialType_UInt,   ///< unsigned int
	dsMaterialType_UVec2,  ///< uvec2
	dsMaterialType_UVec3,  ///< uvec3
	dsMaterialType_UVec4,  ///< uvec4
	dsMaterialType_Bool,   ///< bool
	dsMaterialType_BVec2,  ///< bvec2
	dsMaterialType_BVec3,  ///< bvec3
	dsMaterialType_BVec4,  ///< bvec4

	// Matrices
	dsMaterialType_Mat2,    ///< mat2, mat2x2
	dsMaterialType_Mat3,    ///< mat3, mat3x3
	dsMaterialType_Mat4,    ///< mat4, mat4x4
	dsMaterialType_Mat2x3,  ///< mat2x3
	dsMaterialType_Mat2x4,  ///< mat2x4
	dsMaterialType_Mat3x2,  ///< mat3x2
	dsMaterialType_Mat3x4,  ///< mat3x4
	dsMaterialType_Mat4x2,  ///< mat4x2
	dsMaterialType_Mat4x3,  ///< mat4x3
	dsMaterialType_DMat2,   ///< dmat2, dmat2x2
	dsMaterialType_DMat3,   ///< dmat3, dmat3x3
	dsMaterialType_DMat4,   ///< dmat4, dmat4x4
	dsMaterialType_DMat2x3, ///< dmat2x3
	dsMaterialType_DMat2x4, ///< dmat2x4
	dsMaterialType_DMat3x2, ///< dmat3x2
	dsMaterialType_DMat3x4, ///< dmat3x4
	dsMaterialType_DMat4x2, ///< dmat4x2
	dsMaterialType_DMat4x3, ///< dmat4x3

	// Other types
	dsMaterialType_Texture,       ///< Sampled texture.
	dsMaterialType_Image,         ///< Unsampled image texture.
	dsMaterialType_SubpassInput,  ///< Image result from a previous subpass.
	dsMaterialType_VariableGroup, ///< Group of variables from dsShaderVariableGroup.
	dsMaterialType_UniformBlock,  ///< Graphics buffer bound as a shader block.
	dsMaterialType_UniformBuffer, ///< Graphics buffer bound as a shader buffer.

	dsMaterialType_Count, ///< The number of material types.
} dsMaterialType;

/// \{
typedef struct dsResourceManager dsResourceManager;
typedef struct mslModule mslModule;
typedef struct mslPipeline mslPipeline;
/// \}

/**
 * @brief Struct holding a description of a material.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
 *
 * Implementations should allocate the element list with the material description (ideally with a
 * single allocation) and copy over the elements. The name IDs will be calculated within
 * dsMaterialDesc_create().
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see MaterialDesc.h
 */
typedef struct dsMaterialDesc dsMaterialDesc;

/**
 * @brief Struct holding a description of a shader variable group.
 *
 * This is very similar to dsMaterialDesc, but is used for dsShaderVariableGroup. When shader
 * buffers are supported, the implementation should populate the offsets array.
 *
 * Implementations should allocate the element list with the material description (ideally with a
 * single allocation) and copy over the elements.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see ShaderVariableGroupDesc.h
 */
typedef struct dsShaderVariableGroupDesc dsShaderVariableGroupDesc;

/**
 * @brief Struct for a shader module.
 *
 * A shader module contains the data for multiple shaders. Shaders may then be loaded from the
 * module to render with.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see ShaderModule.h
 */
typedef struct dsShaderModule
{
	/**
	 * @brief The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The underlying module data.
	 *
	 * This is accessed with the ModularShaderLanguage library.
	 */
	mslModule* module;
} dsShaderModule;

/**
 * @brief Struct for a shader.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see Shader.h
 */
typedef struct dsShader
{
	/**
	 * @brief The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The module this shader belongs to.
	 */
	dsShaderModule* module;

	/**
	 * @brief The index of the shader pipeline.
	 */
	uint32_t pipelineIndex;

	/**
	 * @brief The pipeline for the shader.
	 *
	 * This is accessed with the ModularShaderLanguage library.
	 */
	mslPipeline* pipeline;

	/**
	 * @brief A description of the materials that can be used with this shader.
	 */
	const dsMaterialDesc* materialDesc;
} dsShader;

/**
 * @brief Struct describing an element of a material.
 */
typedef struct dsMaterialElement
{
	/**
	 * @brief The name of the element.
	 *
	 * This must remain alive as long as the dsMaterialDesc instance that holds the element.
	 * This can be done by using string literals or holding a table of strings in memory.
	 */
	const char* name;

	/**
	 * @brief The type of the element.
	 */
	dsMaterialType type;

	/**
	 * @brief The number of array elements.
	 *
	 * A count of 0 indicates a non-array.
	 */
	uint32_t count;

	/**
	 * @brief A pointer to the shader variable group description.
	 *
	 * This is only used if type is dsMaterialType_VariableGroup.
	 */
	const dsShaderVariableGroupDesc* shaderVariableGroupDesc;

	/**
	 * @brief Whether or not the variable is volatile, able to change across draw calls.
	 *
	 * This may only be used for shader variables of type:
	 * - dsMaterialType_Texture
	 * - dsMaterialType_Image
	 * - dsMaterialType_VariableGroup
	 * - dsMaterialType_UniformBlock
	 * - dsMaterialType_UniformBuffer
	 */
	bool isVolatile;

	/**
	 * @brief The hash value for the name.
	 *
	 * This will be set when the dsMaterialDesc instance is created and doesn't need to be set by
	 * the calling code.
	 */
	uint32_t nameId;
} dsMaterialElement;

/** @copydoc dsMaterialDesc */
struct dsMaterialDesc
{
	/**
	 * @brief The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The number of material elements.
	 */
	uint32_t elementCount;

	/**
	 * @brief The material elements.
	 */
	dsMaterialElement* elements;
};

/**
 * @brief Struct defining a material to be applied to shaders.
 *
 * Material instances are created with a dsMaterialDesc instance to describe the variables that are
 * set. The values set on this structure will be used to populate the uniforms of a shader.
 *
 * This type is opaque and implemented by the core Render library.
 * @see Material.h
 */
typedef struct dsMaterial dsMaterial;

/**
 * @brief Struct describing an element of a shader variable.
 */
typedef struct dsShaderVariableElement
{
	/**
	 * @brief The name of the element.
	 *
	 * This must remain alive as long as the dsMaterialDesc instance that holds the element.
	 * This can be done by using string literals or holding a table of strings in memory.
	 */
	const char* name;

	/**
	 * @brief The type of the element.
	 */
	dsMaterialType type;

	/**
	 * @brief The number of array elements.
	 *
	 * A count of 0 indicates a non-array.
	 */
	uint32_t count;
} dsShaderVariableElement;

/**
 * @brief Struct describing the position of a shader variable in the final buffer.
 *
 * This is specified by the implementation to be used in the core Render library.
 */
typedef struct dsShaderVariablePos
{
	/**
	 * @brief The offset of the variable in the buffer.
	 */
	uint32_t offset;

	/**
	 * @brief The stride of each array element.
	 */
	uint16_t stride;

	/**
	 * @brief The stride for each column for matrix elements.
	 */
	uint16_t matrixColStride;
} dsShaderVariablePos;

/** @copydoc dsShaderVariableGroupDesc */
struct dsShaderVariableGroupDesc
{
	/**
	 * @brief The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The number of material elements.
	 */
	uint32_t elementCount;

	/**
	 * @brief The shader variable elements.
	 */
	dsShaderVariableElement* elements;

	/**
	 * @brief The position for the elements.
	 *
	 * This is only necessary when shader uniform blocks are supported.
	 */
	dsShaderVariablePos* positions;
};

/**
 * @brief Struct holding a group of shader variables.
 *
 * This type is opaque and implemented by the core Render library.
 *
 * @see ShaderVariableGroup.h
 */
typedef struct dsShaderVariableGroup dsShaderVariableGroup;

/**
 * @brief Struct holding the material values that are marked as volatile.
 *
 * This type is opaque and implemented by the core Render library.
 *
 * @see VolatileMaterialValues.h
 */
typedef struct dsVolatileMaterialValues dsVolatileMaterialValues;

#ifdef __cplusplus
}
#endif
