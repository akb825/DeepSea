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

#include "GLMainCommandBuffer.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "Resources/GLGfxFence.h"
#include "Resources/GLResourceManager.h"
#include "Resources/GLTexture.h"
#include "GLCommandBuffer.h"
#include "GLHelpers.h"
#include "GLRendererInternal.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>

struct dsGLMainCommandBuffer
{
	dsGLCommandBuffer commandBuffer;

	dsGLFenceSyncRef** fenceSyncs;
	size_t curFenceSyncs;
	size_t maxFenceSyncs;
	bool bufferReadback;

	GLuint currentProgram;

	mslRenderState currentState;
	GLuint defaultSamplers[2];
	mslSamplerState defaultSamplerState;
};

static const GLenum polygonModeMap[] =
{
	GL_FLOAT,
	GL_LINE,
	GL_POINT
};

static const GLenum cullFaceMap[] =
{
	GL_FRONT,
	GL_FRONT,
	GL_BACK,
	GL_FRONT_AND_BACK
};

static const GLenum frontFaceMap[] =
{
	GL_CCW,
	GL_CW
};

static const GLenum compareOpMap[] =
{
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS
};

static const GLenum stencilOpMap[] =
{
	GL_KEEP,
	GL_ZERO,
	GL_REPLACE,
	GL_INCR,
	GL_DECR,
	GL_INVERT,
	GL_INCR_WRAP,
	GL_DECR_WRAP
};

static const GLenum logicOpMap[] =
{
	GL_CLEAR,
	GL_AND,
	GL_AND_REVERSE,
	GL_COPY,
	GL_AND_INVERTED,
	GL_NOOP,
	GL_XOR,
	GL_OR,
	GL_NOR,
	GL_EQUIV,
	GL_INVERT,
	GL_OR_REVERSE,
	GL_COPY_INVERTED,
	GL_OR_INVERTED,
	GL_NAND,
	GL_SET
};

static const GLenum blendFactorMap[] =
{
	GL_ZERO,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
	GL_CONSTANT_COLOR,
	GL_ONE_MINUS_CONSTANT_COLOR,
	GL_CONSTANT_ALPHA,
	GL_ONE_MINUS_CONSTANT_ALPHA,
	GL_SRC_ALPHA_SATURATE,
	GL_SRC1_COLOR,
	GL_ONE_MINUS_SRC1_COLOR,
	GL_SRC1_ALPHA,
	GL_ONE_MINUS_SRC1_ALPHA
};

static const GLenum blendOpMap[] =
{
	GL_FUNC_ADD,
	GL_FUNC_SUBTRACT,
	GL_FUNC_REVERSE_SUBTRACT,
	GL_MIN,
	GL_MAX
};

static bool setFences(dsRenderer* renderer, dsGLFenceSyncRef** fenceSyncs, size_t fenceCount,
	bool bufferReadback)
{
	if (ANYGL_SUPPORTED(glMemoryBarrier) && bufferReadback)
		glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

	GLsync glSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	if (!glSync)
	{
		GLenum lastError = dsGetLastGLError();
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error setting fence: %s",
			AnyGL_errorString(lastError));
		errno = dsGetGLErrno(lastError);
	}
	glFlush();

	dsGLFenceSync* sync = dsGLRenderer_createSync(renderer, glSync);
	if (!sync)
	{
		glDeleteSync(glSync);
		return false;
	}

	for (size_t i = 0; i < fenceCount; ++i)
	{
		dsGLFenceSync_addRef(sync);
		DS_ASSERT(!fenceSyncs[i]->sync);
		DS_ATOMIC_STORE_PTR(&fenceSyncs[i]->sync, &sync);
	}

	dsGLFenceSync_freeRef(sync);
	return true;
}

static void resetRasterizationState(mslRasterizationState* state)
{
	state->depthClampEnable = mslBool_False;
	state->rasterizerDiscardEnable = mslBool_False;
	state->polygonMode = mslPolygonMode_Fill;
	state->cullMode = mslCullMode_None;
	state->frontFace = mslFrontFace_CounterClockwise;
	state->depthBiasEnable = mslBool_False;
	state->depthBiasConstantFactor = 0;
	state->depthBiasClamp = 0;
	state->depthBiasSlopeFactor = 0;
	state->lineWidth = 1;

	if (AnyGL_atLeastVersion(3, 2, false) || AnyGL_ARB_depth_clamp)
		glDisable(GL_DEPTH_CLAMP);
	if (ANYGL_SUPPORTED(glPolygonMode))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDisable(GL_DEPTH_BIAS);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0, 0);
	if (AnyGL_EXT_polygon_offset_clamp)
		glDisable(GL_POLYGON_OFFSET_CLAMP_EXT);
	glLineWidth(1.0f);
}

static void resetMultisampleState(mslMultisampleState* state)
{
	state->sampleShadingEnable = mslBool_False;
	state->minSampleShading = 1.0f;
	state->sampleMask = 0xFFFFFFFF;
	state->alphaToCoverageEnable = mslBool_False;
	state->alphaToOneEnable = mslBool_False;

	glEnable(GL_MULTISAMPLE);
	if (ANYGL_SUPPORTED(glMinSampleShading))
	{
		glDisable(GL_SAMPLE_SHADING);
		glMinSampleShading(1.0f);
	}

	if (ANYGL_SUPPORTED(glSampleMaski))
	{
		glDisable(GL_SAMPLE_MASK);
		glSampleMaski(0, 0xFFFFFFFF);
	}

	glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glDisable(GL_SAMPLE_ALPHA_TO_ONE);
}

static void resetStencilState(mslStencilOpState* state)
{
	state->failOp = mslStencilOp_Keep;
	state->passOp = mslStencilOp_Keep;
	state->depthFailOp = mslStencilOp_Keep;
	state->compareOp = mslCompareOp_Always;
	state->compareMask = 0xFFFFFFFF;
	state->writeMask = 0;
	state->reference = 0;
}

static void resetDepthStencilState(mslDepthStencilState* state)
{
	state->depthTestEnable = mslBool_False;
	state->depthWriteEnable = mslBool_False;
	state->depthCompareOp = mslCompareOp_Less;
	state->depthBoundsTestEnable = mslBool_False;
	state->stencilTestEnable = mslBool_False;
	state->minDepthBounds = 0;
	state->maxDepthBounds = 1;

	resetStencilState(&state->frontStencil);
	resetStencilState(&state->frontStencil);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(true);
	glDepthFunc(GL_LESS);
	if (AnyGL_EXT_depth_bounds_test)
	{
		glDisable(GL_DEPTH_BOUNDS_TEST_EXT);
		glDepthBoundsEXT(0, 1);
	}
	glDisable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);
}

static void resetBlendState(mslBlendState* state)
{
	state->logicalOpEnable = mslBool_False;
	state->logicalOp = mslLogicOp_Copy;
	state->separateAttachmentBlendingEnable = mslBool_False;
	for (unsigned int i = 0; i < MSL_MAX_ATTACHMENTS; ++i)
	{
		state->blendAttachments[i].blendEnable = mslBool_False;
		state->blendAttachments[i].srcColorBlendFactor = mslBlendFactor_One;
		state->blendAttachments[i].dstColorBlendFactor = mslBlendFactor_Zero;
		state->blendAttachments[i].colorBlendOp = mslBlendOp_Add;
		state->blendAttachments[i].srcAlphaBlendFactor = mslBlendFactor_One;
		state->blendAttachments[i].dstAlphaBlendFactor = mslBlendFactor_Zero;
		state->blendAttachments[i].alphaBlendOp = mslBlendOp_Add;
		state->blendAttachments[i].colorWriteMask = (mslColorMask)(mslColorMask_Red |
			mslColorMask_Green | mslColorMask_Blue | mslColorMask_Alpha);
	}

	for (unsigned int i = 0; i < 4; ++i)
		state->blendConstants[i] = 0;

	if (ANYGL_SUPPORTED(glLogicOp))
	{
		glDisable(GL_LOGIC_OP);
		glLogicOp(GL_COPY);
	}
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
	glBlendEquation(GL_ADD);
	glColorMask(true, true, true, true);
	glBlendColor(0, 0, 0, 0);
}

static void setRasterizationStates(mslRasterizationState* curState,
	const mslRasterizationState* newState, const dsDynamicRenderStates* dynamicStates)
{
	if (curState->depthClampEnable != newState->depthClampEnable &&
		(AnyGL_atLeastVersion(3, 2, false) || AnyGL_ARB_depth_clamp))
	{
		curState->depthClampEnable = newState->depthClampEnable;
		if (curState->depthClampEnable)
			glEnable(GL_DEPTH_CLAMP);
		else
			glDisable(GL_DEPTH_CLAMP);
	}

	if (curState->polygonMode != newState->polygonMode && ANYGL_SUPPORTED(glPolygonMode))
	{
		curState->polygonMode = curState->polygonMode;
		DS_ASSERT((unsigned int)curState->polygonMode < DS_ARRAY_SIZE(polygonModeMap));
		glPolygonMode(GL_FRONT_AND_BACK, polygonModeMap[curState->polygonMode]);
	}

	if (curState->cullMode != newState->cullMode)
	{
		curState->cullMode = newState->cullMode;
		if (curState->cullMode == mslCullMode_None)
			glDisable(GL_CULL_FACE);
		else
		{
			glEnable(GL_CULL_FACE);
			DS_ASSERT((unsigned int)curState->cullMode < DS_ARRAY_SIZE(cullFaceMap));
			glCullFace(cullFaceMap[curState->cullMode]);
		}
	}

	if (curState->frontFace != newState->frontFace)
	{
		curState->frontFace = newState->frontFace;
		DS_ASSERT((unsigned int)curState->frontFace < DS_ARRAY_SIZE(frontFaceMap));
		glFrontFace(frontFaceMap[curState->frontFace]);
	}

	if (curState->depthBiasEnable != newState->depthBiasEnable)
	{
		curState->depthBiasEnable = newState->depthBiasEnable;
		if (curState->depthBiasEnable)
			glEnable(GL_POLYGON_OFFSET_FILL);
		else
			glDisable(GL_POLYGON_OFFSET_FILL);
	}

	if (curState->depthBiasEnable)
	{
		float constantFactor = 0;
		if (curState->depthBiasConstantFactor != MSL_UNKNOWN_FLOAT)
			constantFactor = curState->depthBiasConstantFactor;
		else if (dynamicStates)
			constantFactor = dynamicStates->depthBiasConstantFactor;

		float clamp = 0;
		if (curState->depthBiasClamp != MSL_UNKNOWN_FLOAT)
			clamp = curState->depthBiasClamp;
		else if (dynamicStates)
			clamp = dynamicStates->depthBiasClamp;

		float slopeFactor = 0;
		if (curState->depthBiasSlopeFactor != MSL_UNKNOWN_FLOAT)
			slopeFactor = curState->depthBiasSlopeFactor;
		else if (dynamicStates)
			slopeFactor = dynamicStates->depthBiasSlopeFactor;

		if (curState->depthBiasConstantFactor != constantFactor ||
			curState->depthBiasClamp != clamp ||
			curState->depthBiasSlopeFactor != slopeFactor)
		{
			curState->depthBiasConstantFactor = constantFactor;
			curState->depthBiasClamp = clamp;
			curState->depthBiasSlopeFactor = slopeFactor;

			if (ANYGL_SUPPORTED(glPolygonOffsetClampEXT))
				glPolygonOffsetClampEXT(slopeFactor, constantFactor, clamp);
			else
				glPolygonOffset(slopeFactor, constantFactor);
		}
	}

	if (curState->lineWidth != newState->lineWidth)
	{
		curState->lineWidth = newState->lineWidth;
		glLineWidth(curState->lineWidth);
	}
}

static void setMultisampleStates(mslMultisampleState* curState,
	const mslMultisampleState* newState)
{
	if (curState->sampleShadingEnable != newState->sampleShadingEnable &&
		ANYGL_SUPPORTED(glMinSampleShading))
	{
		curState->sampleShadingEnable = newState->sampleShadingEnable;
		if (curState->sampleShadingEnable)
			glEnable(GL_SAMPLE_SHADING);
		else
			glDisable(GL_SAMPLE_SHADING);
	}

	if (curState->sampleShadingEnable &&
		curState->minSampleShading != newState->minSampleShading &&
		ANYGL_SUPPORTED(glMinSampleShading))
	{
		curState->minSampleShading = newState->minSampleShading;
		glMinSampleShading(curState->minSampleShading);
	}

	if (curState->sampleMask != newState->sampleMask && ANYGL_SUPPORTED(glSampleMaski))
	{
		curState->sampleMask = newState->sampleMask;
		if (curState->sampleMask == 0xFFFFFFFF)
			glDisable(GL_SAMPLE_MASK);
		else
		{
			glEnable(GL_SAMPLE_MASK);
			glSampleMaski(0, curState->sampleMask);
		}
	}

	if (curState->alphaToCoverageEnable != newState->alphaToCoverageEnable)
	{
		curState->alphaToCoverageEnable = newState->alphaToCoverageEnable;
		if (curState->alphaToCoverageEnable)
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		else
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	}

	if (curState->alphaToOneEnable != newState->alphaToOneEnable)
	{
		curState->alphaToOneEnable = newState->alphaToOneEnable;
		if (curState->alphaToOneEnable)
			glEnable(GL_SAMPLE_ALPHA_TO_ONE);
		else
			glDisable(GL_SAMPLE_ALPHA_TO_ONE);
	}
}

static void setDepthStencilStates(mslDepthStencilState* curState,
	const mslDepthStencilState* newState, const dsDynamicRenderStates* dynamicStates)
{
	if (curState->depthTestEnable != newState->depthTestEnable)
	{
		curState->depthTestEnable = newState->depthTestEnable;
		if (curState->depthTestEnable)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	if (curState->depthWriteEnable != newState->depthWriteEnable)
	{
		curState->depthWriteEnable = newState->depthWriteEnable;
		glDepthMask(curState->depthWriteEnable);
	}

	if (curState->depthTestEnable && curState->depthCompareOp != newState->depthCompareOp)
	{
		curState->depthCompareOp = newState->depthCompareOp;
		DS_ASSERT((unsigned int)curState->depthCompareOp < DS_ARRAY_SIZE(compareOpMap));
		glDepthFunc(compareOpMap[curState->depthCompareOp]);
	}

	if (curState->depthBoundsTestEnable != newState->depthBoundsTestEnable &&
		AnyGL_EXT_depth_bounds_test)
	{
		curState->depthBoundsTestEnable = newState->depthBoundsTestEnable;
		if (curState->depthBoundsTestEnable)
			glEnable(GL_DEPTH_BOUNDS_TEST_EXT);
		else
			glDisable(GL_DEPTH_BOUNDS_TEST_EXT);
	}

	if (curState->depthBoundsTestEnable && AnyGL_EXT_depth_bounds_test)
	{
		float minDepthBounds = 0;
		if (newState->minDepthBounds != MSL_UNKNOWN_FLOAT)
			minDepthBounds = newState->minDepthBounds;
		else if (dynamicStates)
			minDepthBounds = dynamicStates->depthBounds.x;

		float maxDepthBounds = 0;
		if (newState->maxDepthBounds != MSL_UNKNOWN_FLOAT)
			maxDepthBounds = newState->maxDepthBounds;
		else if (dynamicStates)
			maxDepthBounds = dynamicStates->depthBounds.y;

		if (curState->minDepthBounds != minDepthBounds ||
			curState->maxDepthBounds != maxDepthBounds)
		{
			curState->minDepthBounds = minDepthBounds;
			curState->maxDepthBounds = maxDepthBounds;
			glDepthBoundsEXT(minDepthBounds, maxDepthBounds);
		}
	}

	if (curState->stencilTestEnable != newState->stencilTestEnable)
	{
		curState->stencilTestEnable = newState->stencilTestEnable;
		if (curState->stencilTestEnable)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
	}

	if (curState->stencilTestEnable)
	{
		if (curState->frontStencil.failOp != newState->frontStencil.failOp ||
			curState->frontStencil.passOp != newState->frontStencil.passOp ||
			curState->frontStencil.depthFailOp != newState->frontStencil.depthFailOp ||
			curState->backStencil.failOp != newState->backStencil.failOp ||
			curState->backStencil.passOp != newState->backStencil.passOp ||
			curState->backStencil.depthFailOp != newState->backStencil.depthFailOp)
		{
			curState->frontStencil.failOp = newState->frontStencil.failOp;
			curState->frontStencil.passOp = newState->frontStencil.passOp;
			curState->frontStencil.depthFailOp = newState->frontStencil.depthFailOp;
			curState->backStencil.failOp = newState->backStencil.failOp;
			curState->backStencil.passOp = newState->backStencil.passOp;
			curState->backStencil.depthFailOp = newState->backStencil.depthFailOp;

			DS_ASSERT((unsigned int)curState->frontStencil.failOp < DS_ARRAY_SIZE(stencilOpMap));
			DS_ASSERT((unsigned int)curState->frontStencil.passOp < DS_ARRAY_SIZE(stencilOpMap));
			DS_ASSERT((unsigned int)curState->frontStencil.depthFailOp <
				DS_ARRAY_SIZE(stencilOpMap));

			DS_ASSERT((unsigned int)curState->backStencil.failOp < DS_ARRAY_SIZE(stencilOpMap));
			DS_ASSERT((unsigned int)curState->backStencil.passOp < DS_ARRAY_SIZE(stencilOpMap));
			DS_ASSERT((unsigned int)curState->backStencil.depthFailOp <
				DS_ARRAY_SIZE(stencilOpMap));

			if (ANYGL_SUPPORTED(glStencilFuncSeparate) &&
				(curState->frontStencil.failOp != curState->backStencil.failOp ||
				curState->frontStencil.passOp != curState->backStencil.passOp ||
				curState->frontStencil.depthFailOp != curState->backStencil.depthFailOp))
			{
				glStencilOpSeparate(GL_FRONT, stencilOpMap[curState->frontStencil.failOp],
					stencilOpMap[curState->frontStencil.depthFailOp],
					stencilOpMap[curState->frontStencil.passOp]);
				glStencilOpSeparate(GL_BACK, stencilOpMap[curState->backStencil.failOp],
					stencilOpMap[curState->backStencil.depthFailOp],
					stencilOpMap[curState->backStencil.passOp]);
			}
			else
			{
				glStencilOp(stencilOpMap[curState->frontStencil.failOp],
					stencilOpMap[curState->frontStencil.depthFailOp],
					stencilOpMap[curState->frontStencil.passOp]);
			}
		}

		GLbitfield frontCompareMask = 0xFFFFFFFF;
		if (newState->frontStencil.compareMask != MSL_UNKNOWN)
			frontCompareMask = newState->frontStencil.compareMask;
		else if (dynamicStates)
			frontCompareMask = dynamicStates->frontStencilCompareMask;

		GLbitfield frontReference = 0;
		if (newState->frontStencil.reference != MSL_UNKNOWN)
			frontReference = newState->frontStencil.reference;
		else if (dynamicStates)
			frontReference = dynamicStates->frontStencilReference;

		GLbitfield backCompareMask = 0xFFFFFFFF;
		if (newState->backStencil.compareMask != MSL_UNKNOWN)
			backCompareMask = newState->backStencil.compareMask;
		else if (dynamicStates)
			backCompareMask = dynamicStates->backStencilCompareMask;

		GLbitfield backReference = 0;
		if (newState->backStencil.reference != MSL_UNKNOWN)
			backReference = newState->backStencil.reference;
		else if (dynamicStates)
			backReference = dynamicStates->backStencilReference;

		if (curState->frontStencil.compareOp != newState->frontStencil.compareOp ||
			curState->frontStencil.compareMask != frontCompareMask ||
			curState->frontStencil.reference != frontReference ||
			curState->backStencil.compareOp != newState->backStencil.compareOp ||
			curState->backStencil.compareMask != backCompareMask ||
			curState->backStencil.reference != backReference)
		{
			curState->frontStencil.compareOp = newState->frontStencil.compareOp;
			curState->frontStencil.compareMask = frontCompareMask;
			curState->frontStencil.reference = frontReference;
			curState->backStencil.compareOp = newState->backStencil.compareOp;
			curState->backStencil.compareMask = backCompareMask;
			curState->backStencil.reference = backReference;

			DS_ASSERT((unsigned int)curState->frontStencil.compareOp < DS_ARRAY_SIZE(compareOpMap));
			DS_ASSERT((unsigned int)curState->backStencil.compareOp < DS_ARRAY_SIZE(compareOpMap));

			if (ANYGL_SUPPORTED(glStencilFuncSeparate) &&
				(curState->frontStencil.compareOp != curState->backStencil.compareOp ||
				curState->frontStencil.reference != curState->backStencil.reference ||
				curState->frontStencil.compareMask != curState->backStencil.compareMask))
			{
				glStencilFuncSeparate(GL_FRONT, compareOpMap[curState->frontStencil.compareOp],
					curState->frontStencil.reference, curState->frontStencil.compareMask);
				glStencilFuncSeparate(GL_BACK, compareOpMap[curState->backStencil.compareOp],
					curState->backStencil.reference, curState->backStencil.compareMask);
			}
			else
			{
				glStencilFunc(compareOpMap[curState->frontStencil.compareOp],
					curState->frontStencil.reference, curState->frontStencil.compareMask);
			}
		}

		GLbitfield frontWriteMask = 0;
		if (newState->frontStencil.writeMask != MSL_UNKNOWN)
			frontWriteMask = newState->frontStencil.writeMask;
		else if (dynamicStates)
			frontWriteMask = dynamicStates->frontStencilWriteMask;

		GLbitfield backWriteMask = 0;
		if (newState->backStencil.writeMask != MSL_UNKNOWN)
			backWriteMask = newState->backStencil.writeMask;
		else if (dynamicStates)
			backWriteMask = dynamicStates->backStencilWriteMask;

		if (curState->frontStencil.writeMask != frontWriteMask ||
			curState->backStencil.writeMask != backWriteMask)
		{
			curState->frontStencil.writeMask = frontWriteMask;
			curState->backStencil.writeMask = backWriteMask;

			if (ANYGL_SUPPORTED(glStencilMaskSeparate) &&
				curState->frontStencil.writeMask != curState->backStencil.writeMask)
			{
				glStencilMaskSeparate(GL_FRONT, curState->frontStencil.writeMask);
				glStencilMaskSeparate(GL_BACK, curState->backStencil.writeMask);
			}
			else
				glStencilMask(curState->frontStencil.writeMask);
		}
	}
}

static void setBlendStates(const dsRenderer* renderer, mslBlendState* curState,
	const mslBlendState* newState, const dsDynamicRenderStates* dynamicStates)
{
	if (curState->logicalOpEnable != newState->logicalOpEnable && ANYGL_SUPPORTED(glLogicOp))
	{
		curState->logicalOpEnable = newState->logicalOpEnable;
		if (curState->logicalOpEnable)
			glEnable(GL_LOGIC_OP);
		else
			glDisable(GL_LOGIC_OP);
	}

	if (curState->logicalOpEnable && curState->logicalOp != newState->logicalOp &&
		ANYGL_SUPPORTED(glLogicOp))
	{
		curState->logicalOp = newState->logicalOp;
		DS_ASSERT((unsigned int)curState->logicalOp < DS_ARRAY_SIZE(logicOpMap));
		glLogicOp(logicOpMap[curState->logicalOp]);
	}

	bool blendStateChanged = false;
	bool blendEnabled = false;
	bool resetBlend = curState->separateAttachmentBlendingEnable !=
		newState->separateAttachmentBlendingEnable;
	curState->separateAttachmentBlendingEnable = newState->separateAttachmentBlendingEnable;
	if (curState->separateAttachmentBlendingEnable || !ANYGL_SUPPORTED(glBlendFunci))
	{
		blendStateChanged = curState->blendAttachments[0].blendEnable !=
			newState->blendAttachments[0].blendEnable;
		curState->blendAttachments[0].blendEnable = newState->blendAttachments[0].blendEnable;
		blendEnabled = curState->blendAttachments[0].blendEnable;

		if (resetBlend ||
			curState->blendAttachments[0].srcColorBlendFactor !=
				newState->blendAttachments[0].srcColorBlendFactor ||
			curState->blendAttachments[0].dstColorBlendFactor !=
				newState->blendAttachments[0].dstColorBlendFactor ||
			curState->blendAttachments[0].dstColorBlendFactor !=
				newState->blendAttachments[0].dstColorBlendFactor ||
			curState->blendAttachments[0].dstAlphaBlendFactor !=
				newState->blendAttachments[0].dstAlphaBlendFactor)
		{
			curState->blendAttachments[0].srcColorBlendFactor =
				newState->blendAttachments[0].srcColorBlendFactor;
			curState->blendAttachments[0].dstColorBlendFactor =
				newState->blendAttachments[0].dstColorBlendFactor;
			curState->blendAttachments[0].dstColorBlendFactor =
				newState->blendAttachments[0].dstColorBlendFactor;
			curState->blendAttachments[0].dstAlphaBlendFactor =
				newState->blendAttachments[0].dstAlphaBlendFactor;

			DS_ASSERT((unsigned int)curState->blendAttachments[0].srcColorBlendFactor <
				DS_ARRAY_SIZE(blendFactorMap));
			DS_ASSERT((unsigned int)curState->blendAttachments[0].srcAlphaBlendFactor <
				DS_ARRAY_SIZE(blendFactorMap));
			DS_ASSERT((unsigned int)curState->blendAttachments[0].dstColorBlendFactor <
				DS_ARRAY_SIZE(blendFactorMap));
			DS_ASSERT((unsigned int)curState->blendAttachments[0].dstAlphaBlendFactor <
				DS_ARRAY_SIZE(blendFactorMap));

			if (ANYGL_SUPPORTED(glBlendFuncSeparate) &&
				(curState->blendAttachments[0].srcColorBlendFactor !=
					curState->blendAttachments[0].srcAlphaBlendFactor ||
				curState->blendAttachments[0].dstColorBlendFactor !=
					curState->blendAttachments[0].dstAlphaBlendFactor))
			{
				glBlendFuncSeparate(
					blendFactorMap[curState->blendAttachments[0].srcColorBlendFactor],
					blendFactorMap[curState->blendAttachments[0].dstColorBlendFactor],
					blendFactorMap[curState->blendAttachments[0].srcAlphaBlendFactor],
					blendFactorMap[curState->blendAttachments[0].dstAlphaBlendFactor]);
			}
			else
			{
				glBlendFunc(blendFactorMap[curState->blendAttachments[0].srcAlphaBlendFactor],
					blendFactorMap[curState->blendAttachments[0].dstAlphaBlendFactor]);
			}
		}

		if (resetBlend ||
			curState->blendAttachments[0].colorBlendOp !=
				newState->blendAttachments[0].colorBlendOp ||
			curState->blendAttachments[0].alphaBlendOp !=
				newState->blendAttachments[0].alphaBlendOp)
		{
			curState->blendAttachments[0].colorBlendOp = newState->blendAttachments[0].colorBlendOp;
			curState->blendAttachments[0].alphaBlendOp = newState->blendAttachments[0].alphaBlendOp;

			DS_ASSERT((unsigned int)curState->blendAttachments[0].colorBlendOp <
				DS_ARRAY_SIZE(blendOpMap));
			DS_ASSERT((unsigned int)curState->blendAttachments[0].alphaBlendOp <
				DS_ARRAY_SIZE(blendOpMap));

			if (ANYGL_SUPPORTED(glBlendEquationSeparate) &&
				curState->blendAttachments[0].colorBlendOp !=
					curState->blendAttachments[0].alphaBlendOp)
			{
				glBlendEquationSeparate(blendOpMap[curState->blendAttachments[0].colorBlendOp],
					blendOpMap[curState->blendAttachments[0].alphaBlendOp]);
			}
			else
				glBlendEquation(blendOpMap[curState->blendAttachments[0].alphaBlendOp]);
		}

		if (resetBlend ||
			curState->blendAttachments[0].colorWriteMask !=
				newState->blendAttachments[0].colorWriteMask)
		{
			curState->blendAttachments[0].colorWriteMask =
				newState->blendAttachments[0].colorWriteMask;
			mslColorMask colorMask = curState->blendAttachments[0].colorWriteMask;
			glColorMask((colorMask & mslColorMask_Red) != 0, (colorMask & mslColorMask_Green) != 0,
				(colorMask & mslColorMask_Blue) != 0, (colorMask & mslColorMask_Alpha) != 0);
		}
	}
	else
	{
		for (unsigned int i = 0; i < renderer->maxColorAttachments; ++i)
		{
			if (curState->blendAttachments[i].blendEnable !=
				newState->blendAttachments[i].blendEnable)
			{
				blendStateChanged = true;
			}
			curState->blendAttachments[i].blendEnable = newState->blendAttachments[i].blendEnable;
			if (curState->blendAttachments[i].blendEnable)
				blendEnabled = true;

			if (curState->blendAttachments[i].srcColorBlendFactor !=
					newState->blendAttachments[i].srcColorBlendFactor ||
				curState->blendAttachments[i].dstColorBlendFactor !=
					newState->blendAttachments[i].dstColorBlendFactor ||
				curState->blendAttachments[i].dstColorBlendFactor !=
					newState->blendAttachments[i].dstColorBlendFactor ||
				curState->blendAttachments[i].dstAlphaBlendFactor !=
					newState->blendAttachments[i].dstAlphaBlendFactor)
			{
				curState->blendAttachments[i].srcColorBlendFactor =
					newState->blendAttachments[i].srcColorBlendFactor;
				curState->blendAttachments[i].dstColorBlendFactor =
					newState->blendAttachments[i].dstColorBlendFactor;
				curState->blendAttachments[i].dstColorBlendFactor =
					newState->blendAttachments[i].dstColorBlendFactor;
				curState->blendAttachments[i].dstAlphaBlendFactor =
					newState->blendAttachments[i].dstAlphaBlendFactor;

				DS_ASSERT((unsigned int)curState->blendAttachments[i].srcColorBlendFactor <
					DS_ARRAY_SIZE(blendFactorMap));
				DS_ASSERT((unsigned int)curState->blendAttachments[i].srcAlphaBlendFactor <
					DS_ARRAY_SIZE(blendFactorMap));
				DS_ASSERT((unsigned int)curState->blendAttachments[i].dstColorBlendFactor <
					DS_ARRAY_SIZE(blendFactorMap));
				DS_ASSERT((unsigned int)curState->blendAttachments[i].dstAlphaBlendFactor <
					DS_ARRAY_SIZE(blendFactorMap));

				if (curState->blendAttachments[i].srcColorBlendFactor !=
						curState->blendAttachments[i].srcAlphaBlendFactor ||
					curState->blendAttachments[i].dstColorBlendFactor !=
						curState->blendAttachments[i].dstAlphaBlendFactor)
				{
					glBlendFuncSeparatei(i,
						blendFactorMap[curState->blendAttachments[i].srcColorBlendFactor],
						blendFactorMap[curState->blendAttachments[i].dstColorBlendFactor],
						blendFactorMap[curState->blendAttachments[i].srcAlphaBlendFactor],
						blendFactorMap[curState->blendAttachments[i].dstAlphaBlendFactor]);
				}
				else
				{
					glBlendFunci(i,
						blendFactorMap[curState->blendAttachments[i].srcAlphaBlendFactor],
						blendFactorMap[curState->blendAttachments[i].dstAlphaBlendFactor]);
				}
			}

			if (curState->blendAttachments[i].colorBlendOp !=
					newState->blendAttachments[i].colorBlendOp ||
				curState->blendAttachments[i].alphaBlendOp !=
					newState->blendAttachments[i].alphaBlendOp)
			{
				curState->blendAttachments[i].colorBlendOp =
					newState->blendAttachments[i].colorBlendOp;
				curState->blendAttachments[i].alphaBlendOp =
					newState->blendAttachments[i].alphaBlendOp;

				DS_ASSERT((unsigned int)curState->blendAttachments[i].colorBlendOp <
					DS_ARRAY_SIZE(blendOpMap));
				DS_ASSERT((unsigned int)curState->blendAttachments[i].alphaBlendOp <
					DS_ARRAY_SIZE(blendOpMap));

				if (curState->blendAttachments[i].colorBlendOp !=
						curState->blendAttachments[i].alphaBlendOp)
				{
					glBlendEquationSeparatei(i,
						blendOpMap[curState->blendAttachments[i].colorBlendOp],
						blendOpMap[curState->blendAttachments[i].alphaBlendOp]);
				}
				else
					glBlendEquationi(i, blendOpMap[curState->blendAttachments[i].alphaBlendOp]);
			}

			if (curState->blendAttachments[i].colorWriteMask !=
					newState->blendAttachments[i].colorWriteMask)
			{
				curState->blendAttachments[i].colorWriteMask =
					newState->blendAttachments[i].colorWriteMask;
				mslColorMask colorMask = curState->blendAttachments[i].colorWriteMask;
				glColorMaski(i, (colorMask & mslColorMask_Red) != 0,
					(colorMask & mslColorMask_Green) != 0, (colorMask & mslColorMask_Blue) != 0,
					(colorMask & mslColorMask_Alpha) != 0);
			}
		}
	}

	if (resetBlend)
	{
		if (blendEnabled)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}

	dsColor4f blendConstants = {{0, 0, 0, 0}};
	if (newState->blendConstants[0] != MSL_UNKNOWN_FLOAT)
	{
		blendConstants.r = newState->blendConstants[0];
		blendConstants.g = newState->blendConstants[1];
		blendConstants.b = newState->blendConstants[2];
		blendConstants.a = newState->blendConstants[3];
	}
	else if (dynamicStates)
		blendConstants = dynamicStates->blendConstants;

	if (curState->blendConstants[0] != blendConstants.r ||
		curState->blendConstants[1] != blendConstants.g ||
		curState->blendConstants[2] != blendConstants.b ||
		curState->blendConstants[3] != blendConstants.a)
	{
		glBlendColor(curState->blendConstants[0], curState->blendConstants[1],
			curState->blendConstants[2], curState->blendConstants[3]);
	}
}

static void setRenderStates(const dsRenderer* renderer, mslRenderState* curState,
	const mslRenderState* newState, const dsDynamicRenderStates* dynamicStates)
{
	setRasterizationStates(&curState->rasterizationState, &newState->rasterizationState,
		dynamicStates);
	setMultisampleStates(&curState->multisampleState, &newState->multisampleState);
	setDepthStencilStates(&curState->depthStencilState, &newState->depthStencilState,
		dynamicStates);
	setBlendStates(renderer, &curState->blendState, &newState->blendState, dynamicStates);
}

static void updateSamplers(const dsRenderer* renderer, const dsGLShader* shader)
{
	if (AnyGL_EXT_texture_filter_anisotropic &&
		renderer->defaultAnisotropy != shader->defaultAnisotropy)
	{
		for (uint32_t i = 0; i < shader->pipeline.samplerStateCount; ++i)
		{
			if (shader->samplerStates[i].maxAnisotropy == MSL_UNKNOWN_FLOAT)
			{
				glSamplerParameterf(shader->samplerIds[i], GL_TEXTURE_MAX_ANISOTROPY_EXT,
					renderer->defaultAnisotropy);
			}
		}
		((dsGLShader*)shader)->defaultAnisotropy = renderer->defaultAnisotropy;
	}
}

static void setTextureState(const dsRenderer* renderer, const mslSamplerState* samplerState,
	GLenum target, dsGLTexture* texture, bool isShadowSampler)
{
	GLenum curEnum = dsGetGLMinFilter(samplerState->minFilter, samplerState->mipFilter);
	if (texture->minFilter != curEnum)
	{
		glTextureParameteri(target, GL_TEXTURE_MIN_FILTER, curEnum);
		texture->minFilter = curEnum;
	}

	curEnum = dsGetGLMagFilter(samplerState->magFilter);
	if (texture->magFilter != curEnum)
	{
		glTextureParameteri(target, GL_TEXTURE_MAG_FILTER, curEnum);
		texture->magFilter = curEnum;
	}

	curEnum = dsGetGLAddressMode(samplerState->addressModeU);
	if (texture->addressModeS != curEnum)
	{
		glTextureParameteri(target, GL_TEXTURE_WRAP_S, curEnum);
		texture->addressModeS = curEnum;
	}

	curEnum = dsGetGLAddressMode(samplerState->addressModeV);
	if (texture->addressModeT != curEnum)
	{
		glTextureParameteri(target, GL_TEXTURE_WRAP_T, curEnum);
		texture->addressModeT = curEnum;
	}

	if (renderer->resourceManager->maxTextureDepth > 0)
	{
		curEnum = dsGetGLAddressMode(samplerState->addressModeW);
		if (texture->addressModeR != curEnum)
		{
			glTextureParameteri(target, GL_TEXTURE_WRAP_R, curEnum);
			texture->addressModeR = curEnum;
		}
	}

	float curFloat;
	if (AnyGL_EXT_texture_filter_anisotropic)
	{
		curFloat = samplerState->maxAnisotropy == MSL_UNKNOWN_FLOAT ?
			renderer->defaultAnisotropy : samplerState->maxAnisotropy;
		if (texture->anisotropy != curFloat)
		{
			glTextureParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, curFloat);
			texture->anisotropy = curFloat;
		}
	}

	if (AnyGL_atLeastVersion(2, 0, false) || AnyGL_atLeastVersion(3, 0, true))
	{
		curFloat = samplerState->mipLodBias == MSL_UNKNOWN_FLOAT ? 0.0f : samplerState->mipLodBias;
		if (texture->mipLodBias != curFloat)
		{
			glTextureParameterf(target, GL_TEXTURE_LOD_BIAS, curFloat);
			texture->mipLodBias = curFloat;
		}

		curFloat = samplerState->minLod == MSL_UNKNOWN_FLOAT ? -1000.0f : samplerState->minLod;
		if (texture->minLod != curFloat)
		{
			glTextureParameterf(target, GL_TEXTURE_MIN_LOD, curFloat);
			texture->minLod = curFloat;
		}

		curFloat = samplerState->maxLod == MSL_UNKNOWN_FLOAT ? 1000.0f : samplerState->maxLod;
		if (texture->maxLod != curFloat)
		{
			glTextureParameterf(target, GL_TEXTURE_MAX_LOD, curFloat);
			texture->maxLod = curFloat;
		}
	}

	if (AnyGL_atLeastVersion(1, 0, false) || AnyGL_OES_texture_border_clamp)
	{
		if (texture->borderColor != samplerState->borderColor)
		{
			switch (samplerState->borderColor)
			{
				case mslBorderColor_Unset:
				case mslBorderColor_TransparentBlack:
				{
					float color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
					glTextureParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);
					break;
				}
				case mslBorderColor_TransparentIntZero:
				{
					GLint color[4] = {0, 0, 0, 0};
					glTextureParameterIiv(target, GL_TEXTURE_BORDER_COLOR, color);
					break;
				}
				case mslBorderColor_OpaqueBlack:
				{
					float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
					glTextureParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);
					break;
				}
				case mslBorderColor_OpaqueIntZero:
				{
					GLint color[4] = {0, 0, 0, 1};
					glTextureParameterIiv(target, GL_TEXTURE_BORDER_COLOR, color);
					break;
				}
				case mslBorderColor_OpaqueWhite:
				{
					float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
					glTextureParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);
					break;
				}
				case mslBorderColor_OpaqueIntOne:
				{
					GLint color[4] = {1, 1, 1, 1};
					glTextureParameterIiv(target, GL_TEXTURE_BORDER_COLOR, color);
					break;
				}
			}
		}
	}

	if (AnyGL_atLeastVersion(2, 0, false) || AnyGL_atLeastVersion(3, 0, true))
	{
		if (texture->compareEnabled != isShadowSampler)
		{
			glTextureParameteri(target, GL_TEXTURE_COMPARE_MODE,
				isShadowSampler ? GL_COMPARE_R_TO_TEXTURE : GL_NONE);
			texture->compareEnabled = isShadowSampler;
		}

		curEnum = dsGetGLCompareOp(samplerState->compareOp);
		if (texture->compareOp != curEnum)
		{
			glTextureParameteri(target, GL_TEXTURE_COMPARE_FUNC, curEnum);
			texture->compareOp = curEnum;
		}
	}
}

bool dsGLMainCommandBuffer_copyBufferData(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size)
{
	DS_UNUSED(commandBuffer);

	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->bufferId);
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

bool dsGLMainCommandBuffer_copyBuffer(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size)
{
	DS_UNUSED(commandBuffer);

	dsGLGfxBuffer* glSrcBuffer = (dsGLGfxBuffer*)srcBuffer;
	dsGLGfxBuffer* glDstBuffer = (dsGLGfxBuffer*)dstBuffer;
	glCopyBufferSubData(glSrcBuffer->bufferId, glDstBuffer->bufferId, srcOffset, dstOffset, size);
	return true;
}

bool dsGLMainCommandBuffer_copyTextureData(dsCommandBuffer* commandBuffer, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height, uint32_t layers,
	const void* data, size_t size)
{
	DS_UNUSED(commandBuffer);

	dsGLTexture* glTexture = (dsGLTexture*)texture;
	GLenum target = dsGLTexture_target(texture);

	bool compressed = dsGfxFormat_compressedIndex(texture->format) > 0;
	GLenum internalFormat;
	GLenum glFormat;
	GLenum type;
	DS_VERIFY(dsGLResourceManager_getTextureFormatInfo(&internalFormat, &glFormat, &type,
		texture->resourceManager, texture->format));

	glBindTexture(target, glTexture->textureId);
	switch (texture->dimension)
	{
		case dsTextureDim_1D:
			if (texture->depth > 0)
			{
				if (compressed)
				{
					glCompressedTexSubImage2D(GL_TEXTURE_1D_ARRAY, position->mipLevel, position->x,
						position->depth, width, layers, internalFormat, (GLsizei)size, data);
				}
				else
				{
					glTexSubImage2D(GL_TEXTURE_1D_ARRAY, position->mipLevel, position->x,
						position->depth, width, layers, glFormat, type, data);
				}
			}
			else
			{
				if (compressed)
				{
					glCompressedTexSubImage1D(GL_TEXTURE_1D, position->mipLevel, position->x,
						width, internalFormat, (GLsizei)size, data);
				}
				else
				{
					glTexSubImage1D(GL_TEXTURE_1D, position->mipLevel, position->x, width, glFormat,
						type, data);
				}
			}
			break;
		case dsTextureDim_2D:
			if (texture->depth > 0)
			{
				if (compressed)
				{
					glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, position->mipLevel, position->x,
						position->y, position->depth, width, height, layers, internalFormat,
						(GLsizei)size, data);
				}
				else
				{
					glTexSubImage3D(GL_TEXTURE_2D_ARRAY, position->mipLevel, position->x,
						position->y, position->depth, width, height, layers, glFormat, type, data);
				}
			}
			else
			{
				if (compressed)
				{
					glCompressedTexSubImage2D(GL_TEXTURE_2D, position->mipLevel, position->x,
						position->y, width, height, internalFormat, (GLsizei)size, data);
				}
				else
				{
					glTexSubImage2D(GL_TEXTURE_2D, position->mipLevel, position->x, position->y,
						width, height, glFormat, type, data);
				}
			}
			break;
		case dsTextureDim_3D:
			if (compressed)
			{
				glCompressedTexSubImage3D(GL_TEXTURE_3D, position->mipLevel, position->x,
					position->y, position->depth, width, height, layers, internalFormat,
					(GLsizei)size, data);
			}
			else
			{
				glTexSubImage3D(GL_TEXTURE_3D, position->mipLevel, position->x, position->y,
					position->depth, width, height, layers, glFormat, type, data);
			}
			break;
		case dsTextureDim_Cube:
			if (texture->depth > 0)
			{
				if (compressed)
				{
					glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, position->mipLevel,
						position->x, position->y, position->depth*6 + position->face, width, height,
						layers, internalFormat, (GLsizei)size, data);
				}
				else
				{
					glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, position->mipLevel, position->x,
						position->y, position->depth*6 + position->face, width, height, layers,
						glFormat, type, data);
				}
			}
			else
			{
				for (unsigned int j = 0; j < layers; ++j)
				{
					if (compressed)
					{
						glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + position->face,
							position->mipLevel, position->x, position->y, width, height,
							internalFormat, (GLsizei)size, data);
					}
					else
					{
						glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + position->face,
							position->mipLevel, position->x, position->y, width, height, glFormat,
							type, data);
					}
				}
			}
			break;
		default:
			DS_ASSERT(false);
	}
	glBindTexture(target, 0);

	return true;
}

bool dsGLMainCommandBuffer_copyTexture(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsTexture* dstTexture, const dsTextureCopyRegion* regions, size_t regionCount)
{
	DS_UNUSED(commandBuffer);

	dsGLTexture* glSrcTexture = (dsGLTexture*)srcTexture;
	dsGLTexture* glDstTexture = (dsGLTexture*)dstTexture;
	if (ANYGL_SUPPORTED(glCopyImageSubData))
	{
		GLenum srcTarget = dsGLTexture_copyTarget(srcTexture);
		GLenum dstTarget = dsGLTexture_copyTarget(dstTexture);

		for (size_t i = 0; i < regionCount; ++i)
		{
			uint32_t srcLayer = regions[i].srcPosition.depth;
			if (srcTexture->dimension == dsTextureDim_Cube)
				srcLayer = srcLayer*6 + regions[i].dstPosition.face;
			uint32_t dstLayer = regions[i].dstPosition.depth;
			if (dstTexture->dimension == dsTextureDim_Cube)
				dstLayer = dstLayer*6 + regions[i].dstPosition.face;

			glCopyImageSubData(glSrcTexture->textureId, srcTarget, regions[i].srcPosition.mipLevel,
				regions[i].srcPosition.x, regions[i].srcPosition.y, srcLayer,
				glDstTexture->textureId, dstTarget, regions[i].dstPosition.mipLevel,
				regions[i].dstPosition.x, regions[i].dstPosition.y, dstLayer, regions[i].width,
				regions[i].height, regions[i].layers);
		}
	}
	else
	{
		dsRenderer* renderer = commandBuffer->renderer;
		GLuint tempFramebuffer = dsGLRenderer_tempFramebuffer(renderer);
		GLuint tempCopyFramebuffer = dsGLRenderer_tempCopyFramebuffer(renderer);
		if (!tempFramebuffer || !tempCopyFramebuffer)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
				"Texture blitting may only be done during rendering.");
			return false;
		}

		glBindFramebuffer(GL_READ_FRAMEBUFFER, tempFramebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempCopyFramebuffer);

		GLbitfield buffers = dsGLTexture_buffers(srcTexture);
		for (uint32_t i = 0; i < regionCount; ++i)
		{
			uint32_t srcLayer = regions[i].srcPosition.depth;
			if (srcTexture->dimension == dsTextureDim_Cube)
				srcLayer = srcLayer*6 + regions[i].dstPosition.face;
			uint32_t dstLayer = regions[i].dstPosition.depth;
			if (dstTexture->dimension == dsTextureDim_Cube)
				dstLayer = dstLayer*6 + regions[i].dstPosition.face;

			for (uint32_t j = 0; j < regions[i].layers; ++j)
			{
				dsGLBindFramebufferTexture(GL_READ_FRAMEBUFFER, srcTexture,
					regions[i].srcPosition.mipLevel, srcLayer + j);
				dsGLBindFramebufferTexture(GL_DRAW_FRAMEBUFFER, dstTexture,
					regions[i].dstPosition.mipLevel, dstLayer + j);
				glBlitFramebuffer(regions[i].srcPosition.x, regions[i].srcPosition.y,
					regions[i].srcPosition.x + regions[i].width,
					regions[i].srcPosition.y + regions[i].height,
					regions[i].dstPosition.x, regions[i].dstPosition.y,
					regions[i].dstPosition.x + regions[i].width,
					regions[i].dstPosition.y + regions[i].height, buffers, GL_NEAREST);
			}
		}

		dsGLUnbindFramebufferTexture(GL_READ_FRAMEBUFFER, srcTexture);
		dsGLUnbindFramebufferTexture(GL_DRAW_FRAMEBUFFER, dstTexture);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	return true;
}

bool dsGLMainCommandBuffer_blitTexture(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsTexture* dstTexture, const dsTextureBlitRegion* regions, size_t regionCount,
	dsBlitFilter filter)
{
	DS_UNUSED(commandBuffer);

	dsRenderer* renderer = commandBuffer->renderer;
	GLuint tempFramebuffer = dsGLRenderer_tempFramebuffer(renderer);
	GLuint tempCopyFramebuffer = dsGLRenderer_tempCopyFramebuffer(renderer);
	if (!tempFramebuffer || !tempCopyFramebuffer)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Texture blitting may only be done during rendering.");
		return false;
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, tempFramebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempCopyFramebuffer);

	GLbitfield buffers = dsGLTexture_buffers(srcTexture);
	for (uint32_t i = 0; i < regionCount; ++i)
	{
		uint32_t srcLayer = regions[i].srcPosition.depth;
		if (srcTexture->dimension == dsTextureDim_Cube)
			srcLayer = srcLayer*6 + regions[i].dstPosition.face;
		uint32_t dstLayer = regions[i].dstPosition.depth;
		if (dstTexture->dimension == dsTextureDim_Cube)
			dstLayer = dstLayer*6 + regions[i].dstPosition.face;

		for (uint32_t j = 0; j < regions[i].layers; ++j)
		{
			dsGLBindFramebufferTexture(GL_READ_FRAMEBUFFER, srcTexture,
				regions[i].srcPosition.mipLevel, srcLayer + j);
			dsGLBindFramebufferTexture(GL_DRAW_FRAMEBUFFER, dstTexture,
				regions[i].dstPosition.mipLevel, dstLayer + j);
			glBlitFramebuffer(regions[i].srcPosition.x, regions[i].srcPosition.y,
				regions[i].srcPosition.x + regions[i].srcWidth,
				regions[i].srcPosition.y + regions[i].srcHeight,
				regions[i].dstPosition.x, regions[i].dstPosition.y,
				regions[i].dstPosition.x + regions[i].dstWidth,
				regions[i].dstPosition.y + regions[i].dstHeight, buffers,
				filter == dsBlitFilter_Linear ? GL_LINEAR : GL_NEAREST);
		}
	}

	dsGLUnbindFramebufferTexture(GL_READ_FRAMEBUFFER, srcTexture);
	dsGLUnbindFramebufferTexture(GL_DRAW_FRAMEBUFFER, dstTexture);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	return true;
}

bool dsGLMainCommandBuffer_setFenceSyncs(dsCommandBuffer* commandBuffer, dsGLFenceSyncRef** syncs,
	size_t syncCount, bool bufferReadback)
{
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	if (((dsGLCommandBuffer*)commandBuffer)->insideRenderPass)
	{
		size_t index = glCommandBuffer->curFenceSyncs;
		if (!dsGLAddToBuffer(commandBuffer->allocator, (void**)&glCommandBuffer->fenceSyncs,
			&glCommandBuffer->curFenceSyncs, &glCommandBuffer->maxFenceSyncs,
			sizeof(dsGLFenceSyncRef*), syncCount))
		{
			return false;
		}

		DS_ASSERT(index + syncCount <= glCommandBuffer->maxFenceSyncs);
		for (size_t i = 0; i < syncCount; ++i)
		{
			glCommandBuffer->fenceSyncs[index + i] = syncs[i];
			dsGLFenceSyncRef_addRef(syncs[i]);
		}
		glCommandBuffer->curFenceSyncs += syncCount;

		if (bufferReadback)
			glCommandBuffer->bufferReadback = bufferReadback;

		return true;
	}
	else
		return setFences(commandBuffer->renderer, syncs, syncCount, bufferReadback);
}

bool dsGLMainCommandBuffer_bindShader(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsDynamicRenderStates* renderStates)
{
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	const dsGLShader* glShader = (const dsGLShader*)shader;
	if (glCommandBuffer->currentProgram != glShader->programId)
	{
		glUseProgram(glShader->programId);
		glCommandBuffer->currentProgram = glShader->programId;
	}

	setRenderStates(commandBuffer->renderer, &glCommandBuffer->currentState, &glShader->renderState,
		renderStates);
	updateSamplers(commandBuffer->renderer, glShader);
	return true;
}

bool dsGLMainCommandBuffer_setTexture(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsTexture* texture)
{
	dsGLMainCommandBuffer* glCommandBuffer = (dsGLMainCommandBuffer*)commandBuffer;
	const dsGLShader* glShader = (const dsGLShader*)shader;
	dsGLTexture* glTexture = (dsGLTexture*)texture;

	uint32_t textureIndex = glShader->uniforms[element].location;
	uint32_t samplerIndex = glShader->uniforms[element].samplerIndex;
	GLuint textureId = glTexture ? glTexture->textureId : 0;
	glActiveTexture(GL_TEXTURE0 + textureIndex);
	GLenum target = dsGLTexture_target(texture);
	glBindTexture(target, textureId);

	bool isShadowSampler = glShader->uniforms[element].isShadowSampler != 0;
	if (ANYGL_SUPPORTED(glBindSampler))
	{
		if (samplerIndex == MSL_UNKNOWN)
			glBindSampler(textureIndex, glCommandBuffer->defaultSamplers[isShadowSampler]);
		else
			glBindSampler(textureIndex, glShader->samplerIds[samplerIndex]);
	}
	else if (glTexture)
	{
		setTextureState(commandBuffer->renderer, glShader->samplerStates + samplerIndex,
			target, glTexture, isShadowSampler);
	}

	return true;
}

bool dsGLMainCommandBuffer_setTextureBuffer(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsGfxBuffer* buffer, dsGfxFormat format, size_t offset, size_t count)
{
	DS_UNUSED(commandBuffer);
	DS_ASSERT(buffer);
	const dsGLShader* glShader = (const dsGLShader*)shader;
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;
	GLenum internalFormat;
	DS_VERIFY(dsGLResourceManager_getTextureFormatInfo(&internalFormat, NULL, NULL,
		shader->resourceManager, format));

	uint32_t textureIndex = glShader->uniforms[element].location;
	glActiveTexture(GL_TEXTURE0 + textureIndex);
	if (ANYGL_SUPPORTED(glTexBufferRange))
	{
		glTexBufferRange(GL_TEXTURE_BUFFER, internalFormat, glBuffer->bufferId, offset,
			dsGfxFormat_size(format)*count);
	}
	else
		glTextureBuffer(GL_TEXTURE_BUFFER, internalFormat, glBuffer->bufferId);

	return true;
}

bool dsGLMainCommandBuffer_setShaderBuffer(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsGfxBuffer* buffer, size_t offset, size_t size)
{
	DS_UNUSED(commandBuffer);
	const dsGLShader* glShader = (const dsGLShader*)shader;
	dsGLGfxBuffer* glBuffer = (dsGLGfxBuffer*)buffer;

	GLenum type = 0;
	switch (shader->materialDesc->elements[element].type)
	{
		case dsMaterialType_UniformBlock:
		case dsMaterialType_VariableGroup:
			type = GL_UNIFORM_BUFFER;
			break;
		case dsMaterialType_UniformBuffer:
			type = GL_SHADER_STORAGE_BUFFER;
			break;
		default:
			DS_ASSERT(false);
	}

	glBindBufferRange(type, glShader->uniforms[element].location,
		glBuffer ? glBuffer->bufferId : 0, offset, size);

	return true;
}

bool dsGLMainCommandBuffer_setUniform(dsCommandBuffer* commandBuffer, GLint location,
	dsMaterialType type, uint32_t count, const void* data)
{
	DS_UNUSED(commandBuffer);
	count = dsMax(1U, count);
	// Compiling and getting the uniform locations should have already given errors for unsupporte
	// types, so shouldn't have to do error checking here.
	switch (type)
	{
		case dsMaterialType_Float:
			glUniform1fv(location, count, (const float*)data);
			break;
		case dsMaterialType_Vec2:
			glUniform2fv(location, count, (const float*)data);
			break;
		case dsMaterialType_Vec3:
			glUniform3fv(location, count, (const float*)data);
			break;
		case dsMaterialType_Vec4:
			glUniform4fv(location, count, (const float*)data);
			break;
		case dsMaterialType_Double:
			glUniform1dv(location, count, (const double*)data);
			break;
		case dsMaterialType_DVec2:
			glUniform2dv(location, count, (const double*)data);
			break;
		case dsMaterialType_DVec3:
			glUniform3dv(location, count, (const double*)data);
			break;
		case dsMaterialType_DVec4:
			glUniform4dv(location, count, (const double*)data);
			break;
		case dsMaterialType_Int:
		case dsMaterialType_Bool:
			glUniform1iv(location, count, (const int*)data);
			break;
		case dsMaterialType_IVec2:
		case dsMaterialType_BVec2:
			glUniform2iv(location, count, (const int*)data);
			break;
		case dsMaterialType_IVec3:
		case dsMaterialType_BVec3:
			glUniform3iv(location, count, (const int*)data);
			break;
		case dsMaterialType_IVec4:
		case dsMaterialType_BVec4:
			glUniform4iv(location, count, (const int*)data);
			break;
		case dsMaterialType_UInt:
			glUniform1uiv(location, count, (const unsigned int*)data);
			break;
		case dsMaterialType_UVec2:
			glUniform2uiv(location, count, (const unsigned int*)data);
			break;
		case dsMaterialType_UVec3:
			glUniform3uiv(location, count, (const unsigned int*)data);
			break;
		case dsMaterialType_UVec4:
			glUniform4uiv(location, count, (const unsigned int*)data);
			break;
		case dsMaterialType_Mat2:
			glUniformMatrix2fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat3:
			glUniformMatrix3fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat4:
			glUniformMatrix4fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat2x3:
			glUniformMatrix2x3fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat2x4:
			glUniformMatrix2x4fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat3x2:
			glUniformMatrix3x2fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat3x4:
			glUniformMatrix3x4fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat4x2:
			glUniformMatrix4x2fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_Mat4x3:
			glUniformMatrix4x3fv(location, count, false, (const float*)data);
			break;
		case dsMaterialType_DMat2:
			glUniformMatrix2dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat3:
			glUniformMatrix3dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat4:
			glUniformMatrix4dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat2x3:
			glUniformMatrix2x3dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat2x4:
			glUniformMatrix2x3dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat3x2:
			glUniformMatrix3x2dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat3x4:
			glUniformMatrix3x4dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat4x2:
			glUniformMatrix4x2dv(location, count, false, (const double*)data);
			break;
		case dsMaterialType_DMat4x3:
			glUniformMatrix4x3dv(location, count, false, (const double*)data);
			break;
		default:
			DS_ASSERT(false);
	}

	return true;
}

bool dsGLMainCommandBuffer_unbindShader(dsCommandBuffer* commandBuffer, const dsShader* shader)
{
	DS_UNUSED(commandBuffer);
	DS_UNUSED(shader);
	return true;
}

bool dsGLMainCommandBuffer_beginRenderSurface(dsCommandBuffer* commandBuffer, void* glSurface)
{
	return dsGLRenderer_bindSurface(commandBuffer->renderer, glSurface);
}

bool dsGLMainCommandBuffer_endRenderSurface(dsCommandBuffer* commandBuffer, void* glSurface)
{
	DS_UNUSED(commandBuffer);
	DS_UNUSED(glSurface);
	return true;
}

bool dsGLMainCommandBuffer_begin(dsCommandBuffer* commandBuffer, const dsRenderPass* renderPass,
	uint32_t subpassIndex, const dsFramebuffer* framebuffer)
{
	DS_UNUSED(commandBuffer);
	DS_UNUSED(renderPass);
	DS_UNUSED(subpassIndex);
	DS_UNUSED(framebuffer);

	errno = EPERM;
	DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Cannot begin or end the main command buffer.");
	return false;
}

bool dsGLMainCommandBuffer_end(dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);

	errno = EPERM;
	DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Cannot begin or end the main command buffer.");
	return false;
}

bool dsGLMainCommandBuffer_submit(dsCommandBuffer* commandBuffer, dsCommandBuffer* submitBuffer)
{
	DS_UNUSED(commandBuffer);
	DS_UNUSED(submitBuffer);
	errno = EPERM;
	DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Cannot submit the main command buffer.");
	return false;
}

static CommandBufferFunctionTable functionTable =
{
	&dsGLMainCommandBuffer_copyBufferData,
	&dsGLMainCommandBuffer_copyBuffer,
	&dsGLMainCommandBuffer_copyTextureData,
	&dsGLMainCommandBuffer_copyTexture,
	&dsGLMainCommandBuffer_blitTexture,
	&dsGLMainCommandBuffer_setFenceSyncs,
	&dsGLMainCommandBuffer_bindShader,
	&dsGLMainCommandBuffer_setTexture,
	&dsGLMainCommandBuffer_setTextureBuffer,
	&dsGLMainCommandBuffer_setShaderBuffer,
	&dsGLMainCommandBuffer_setUniform,
	&dsGLMainCommandBuffer_unbindShader,
	&dsGLMainCommandBuffer_beginRenderSurface,
	&dsGLMainCommandBuffer_endRenderSurface,
	&dsGLMainCommandBuffer_begin,
	&dsGLMainCommandBuffer_end,
	&dsGLMainCommandBuffer_submit
};

dsGLMainCommandBuffer* dsGLMainCommandBuffer_create(dsRenderer* renderer, dsAllocator* allocator)
{
	DS_ASSERT(allocator->freeFunc);
	dsGLMainCommandBuffer* commandBuffer = (dsGLMainCommandBuffer*)dsAllocator_alloc(allocator,
		sizeof(dsGLMainCommandBuffer));
	if (!commandBuffer)
		return NULL;

	dsCommandBuffer* baseCommandBuffer = (dsCommandBuffer*)commandBuffer;
	baseCommandBuffer->renderer = renderer;
	baseCommandBuffer->allocator = allocator;
	baseCommandBuffer->usage = dsCommandBufferUsage_Standard;

	((dsGLCommandBuffer*)commandBuffer)->functions = &functionTable;
	commandBuffer->fenceSyncs = NULL;
	commandBuffer->curFenceSyncs = 0;
	commandBuffer->maxFenceSyncs = 0;
	commandBuffer->bufferReadback = false;

	commandBuffer->currentProgram = 0;
	if (ANYGL_SUPPORTED(glGenSamplers))
	{
		glGenSamplers(2, commandBuffer->defaultSamplers);
		glSamplerParameteri(commandBuffer->defaultSamplers[1], GL_TEXTURE_COMPARE_MODE,
			GL_COMPARE_R_TO_TEXTURE);
	}

	commandBuffer->defaultSamplerState.minFilter = mslFilter_Unset;
	commandBuffer->defaultSamplerState.magFilter = mslFilter_Unset;
	commandBuffer->defaultSamplerState.mipFilter = mslMipFilter_Unset;
	commandBuffer->defaultSamplerState.addressModeU = mslAddressMode_Unset;
	commandBuffer->defaultSamplerState.addressModeV = mslAddressMode_Unset;
	commandBuffer->defaultSamplerState.addressModeW = mslAddressMode_Unset;
	commandBuffer->defaultSamplerState.mipLodBias = MSL_UNKNOWN_FLOAT;
	commandBuffer->defaultSamplerState.minLod = MSL_UNKNOWN_FLOAT;
	commandBuffer->defaultSamplerState.maxLod = MSL_UNKNOWN_FLOAT;
	commandBuffer->defaultSamplerState.borderColor = mslBorderColor_Unset;
	commandBuffer->defaultSamplerState.compareOp = mslCompareOp_Unset;

	dsGLCommandBuffer_initialize(baseCommandBuffer);
	dsGLMainCommandBuffer_resetState(commandBuffer);

	return commandBuffer;
}

void dsGLMainCommandBuffer_resetState(dsGLMainCommandBuffer* commandBuffer)
{
	resetRasterizationState(&commandBuffer->currentState.rasterizationState);
	resetMultisampleState(&commandBuffer->currentState.multisampleState);
	resetDepthStencilState(&commandBuffer->currentState.depthStencilState);
	resetBlendState(&commandBuffer->currentState.blendState);

	commandBuffer->currentState.patchControlPoints = MSL_UNKNOWN;
	if (AnyGL_atLeastVersion(3, 2, false) || AnyGL_ARB_seamless_cube_map)
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
}

bool dsGLMainCommandBuffer_destroy(dsGLMainCommandBuffer* commandBuffer)
{
	if (!commandBuffer)
		return true;

	dsAllocator* allocator = ((dsCommandBuffer*)commandBuffer)->allocator;

	if (commandBuffer->fenceSyncs)
	{
		for (size_t i = 0; i < commandBuffer->curFenceSyncs; ++i)
			dsGLFenceSyncRef_freeRef(commandBuffer->fenceSyncs[i]);
		DS_VERIFY(dsAllocator_free(allocator, commandBuffer->fenceSyncs));
	}

	if (ANYGL_SUPPORTED(glDeleteSamplers))
		glDeleteSamplers(2, commandBuffer->defaultSamplers);

	dsGLCommandBuffer_shutdown((dsCommandBuffer*)commandBuffer);
	DS_VERIFY(dsAllocator_free(allocator, commandBuffer));
	return true;
}
