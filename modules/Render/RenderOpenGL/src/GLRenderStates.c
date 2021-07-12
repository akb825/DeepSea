/*
 * Copyright 2017-2021 Aaron Barany
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

#include "GLRenderStates.h"
#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "GLTypes.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <string.h>

static const GLenum polygonModeMap[] =
{
	GL_FILL,
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
}

static void resetMultisampleState(mslMultisampleState* state)
{
	state->sampleShadingEnable = mslBool_False;
	state->minSampleShading = 1.0f;
	state->sampleMask = 0xFFFFFFFF;
	state->alphaToCoverageEnable = mslBool_False;
	state->alphaToOneEnable = mslBool_False;
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
	state->depthWriteEnable = mslBool_True;
	state->depthCompareOp = mslCompareOp_Less;
	state->depthBoundsTestEnable = mslBool_False;
	state->stencilTestEnable = mslBool_False;
	state->minDepthBounds = 0;
	state->maxDepthBounds = 1;

	resetStencilState(&state->frontStencil);
	resetStencilState(&state->frontStencil);
}

static void resetBlendState(mslBlendState* state)
{
	state->logicalOpEnable = mslBool_False;
	state->logicalOp = mslLogicOp_Copy;
	state->separateAttachmentBlendingEnable = mslBool_False;
	for (unsigned int i = 0; i < DS_MAX_ATTACHMENTS; ++i)
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
}

static void setRasterizationStates(const dsResourceManager* resourceManager,
	mslRasterizationState* curState, const mslRasterizationState* newState,
	const dsDynamicRenderStates* dynamicStates, bool invertY, bool dynamicOnly)
{
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

			if (ANYGL_SUPPORTED(glPolygonOffsetClamp))
				glPolygonOffsetClamp(slopeFactor, constantFactor, clamp);
			else
				glPolygonOffset(slopeFactor, constantFactor);
		}
	}

	float lineWidth = 1.0f;
	if (curState->lineWidth != MSL_UNKNOWN_FLOAT)
		lineWidth = curState->lineWidth;
	else if (dynamicStates)
		lineWidth = dynamicStates->lineWidth;
	if (curState->lineWidth != newState->lineWidth)
	{
		curState->lineWidth = lineWidth;
		glLineWidth(dsClamp(lineWidth, resourceManager->lineWidthRange.x,
			resourceManager->lineWidthRange.y));
	}

	if (dynamicOnly)
		return;

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
		curState->polygonMode = newState->polygonMode;
		DS_ASSERT((unsigned int)curState->polygonMode < DS_ARRAY_SIZE(polygonModeMap));
		glPolygonMode(GL_FRONT_AND_BACK, polygonModeMap[curState->polygonMode]);
	}

	// Need to reverse cull mode when Y is inverted.
	mslCullMode adjustedCull = newState->cullMode;
	if (invertY)
	{
		if (adjustedCull == mslCullMode_Front)
			adjustedCull = mslCullMode_Back;
		else if (adjustedCull == mslCullMode_Back)
			adjustedCull = mslCullMode_Front;
	}

	if (curState->cullMode != adjustedCull)
	{
		curState->cullMode = adjustedCull;
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

	if (AnyGL_atLeastVersion(1, 3, false))
	{
		if (curState->alphaToOneEnable != newState->alphaToOneEnable)
		{
			curState->alphaToOneEnable = newState->alphaToOneEnable;
			if (curState->alphaToOneEnable)
				glEnable(GL_SAMPLE_ALPHA_TO_ONE);
			else
				glDisable(GL_SAMPLE_ALPHA_TO_ONE);
		}
	}
}

static void setDepthStencilStates(mslDepthStencilState* curState,
	const mslDepthStencilState* newState, const dsDynamicRenderStates* dynamicStates,
	bool dynamicOnly)
{
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

	if (dynamicOnly)
		return;

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
		glDepthMask((GLboolean)curState->depthWriteEnable);
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
}

static void setBlendStates(const dsRenderer* renderer, mslBlendState* curState,
	const mslBlendState* newState, const dsDynamicRenderStates* dynamicStates, bool dynamicOnly)
{
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
		memcpy(curState->blendConstants, &blendConstants, sizeof(blendConstants));
		glBlendColor(curState->blendConstants[0], curState->blendConstants[1],
			curState->blendConstants[2], curState->blendConstants[3]);
	}

	if (dynamicOnly)
		return;

	if (curState->logicalOpEnable != newState->logicalOpEnable && ANYGL_SUPPORTED(glLogicOp))
	{
		curState->logicalOpEnable = newState->logicalOpEnable;
		if (curState->logicalOpEnable)
			glEnable(GL_COLOR_LOGIC_OP);
		else
			glDisable(GL_COLOR_LOGIC_OP);
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
				curState->blendAttachments[i].srcAlphaBlendFactor !=
					newState->blendAttachments[i].srcAlphaBlendFactor ||
				curState->blendAttachments[i].dstAlphaBlendFactor !=
					newState->blendAttachments[i].dstAlphaBlendFactor)
			{
				curState->blendAttachments[i].srcColorBlendFactor =
					newState->blendAttachments[i].srcColorBlendFactor;
				curState->blendAttachments[i].dstColorBlendFactor =
					newState->blendAttachments[i].dstColorBlendFactor;
				curState->blendAttachments[i].srcAlphaBlendFactor =
					newState->blendAttachments[i].srcAlphaBlendFactor;
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

	if (resetBlend || blendStateChanged)
	{
		if (blendEnabled)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}
}

void dsGLRenderStates_initialize(mslRenderState* state)
{
	resetRasterizationState(&state->rasterizationState);
	resetMultisampleState(&state->multisampleState);
	resetDepthStencilState(&state->depthStencilState);
	resetBlendState(&state->blendState);
	state->patchControlPoints = MSL_UNKNOWN;
	state->clipDistanceCount = 0;
	state->cullDistanceCount = 0;
}

void dsGLRenderStates_updateGLState(const dsRenderer* renderer, mslRenderState* curState,
	const mslRenderState* newState, const dsDynamicRenderStates* dynamicStates)
{
	const dsGLRenderer* glRenderer = (const dsGLRenderer*)renderer;
	setRasterizationStates(renderer->resourceManager, &curState->rasterizationState,
		&newState->rasterizationState, dynamicStates,
		glRenderer->curSurfaceType == GLSurfaceType_Framebuffer, false);
	setMultisampleStates(&curState->multisampleState, &newState->multisampleState);
	setDepthStencilStates(&curState->depthStencilState, &newState->depthStencilState,
		dynamicStates, false);
	setBlendStates(renderer, &curState->blendState, &newState->blendState, dynamicStates, false);

	if (newState->patchControlPoints != MSL_UNKNOWN &&
		curState->patchControlPoints != newState->patchControlPoints &&
		ANYGL_SUPPORTED(glPatchParameteri))
	{
		glPatchParameteri(GL_PATCH_VERTICES, newState->patchControlPoints);
		curState->patchControlPoints = newState->patchControlPoints;
	}

	// NOTE: Should have already prevented any shaders with clip distances from being loaded if not
	// supported.
	if (newState->clipDistanceCount > curState->clipDistanceCount)
	{
		DS_ASSERT(AnyGL_atLeastVersion(3, 0, false));
		for (uint32_t i = curState->clipDistanceCount; i < newState->clipDistanceCount; ++i)
			glEnable(GL_CLIP_DISTANCE0 + i);
		curState->clipDistanceCount = newState->clipDistanceCount;
	}
	else if (newState->clipDistanceCount < curState->clipDistanceCount)
	{
		DS_ASSERT(AnyGL_atLeastVersion(3, 0, false));
		for (uint32_t i = newState->clipDistanceCount; i < curState->clipDistanceCount; ++i)
			glDisable(GL_CLIP_DISTANCE0 + i);
		curState->clipDistanceCount = newState->clipDistanceCount;
	}
}

void dsGLRenderStates_updateDynamicGLStates(const dsRenderer* renderer, mslRenderState* curState,
	const mslRenderState* newState, const dsDynamicRenderStates* dynamicStates)
{
	const dsGLRenderer* glRenderer = (const dsGLRenderer*)renderer;
	setRasterizationStates(renderer->resourceManager, &curState->rasterizationState,
		&newState->rasterizationState, dynamicStates,
		glRenderer->curSurfaceType == GLSurfaceType_Framebuffer, true);
	setDepthStencilStates(&curState->depthStencilState, &newState->depthStencilState,
		dynamicStates, true);
	setBlendStates(renderer, &curState->blendState, &newState->blendState, dynamicStates, true);
}
