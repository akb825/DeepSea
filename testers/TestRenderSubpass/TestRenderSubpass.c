/*
 * Copyright 2017-2024 Aaron Barany
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

#include <DeepSea/Application/Application.h>
#include <DeepSea/Application/Window.h>
#include <DeepSea/ApplicationSDL/SDLApplication.h>

#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/Renderbuffer.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/TextureData.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Render/RenderSurface.h>

#include <DeepSea/RenderBootstrap/RenderBootstrap.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAMPLE_COUNT 4
#define NO_BLIT DS_MAC

typedef struct TestRenderSubpass
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsWindow* window;
	dsFramebuffer* framebuffer;

	dsRenderPass* renderPass;
	dsShaderModule* shaderModule;
	dsShaderVariableGroupDesc* transformGroupDesc;
	dsMaterialDesc* cubeMaterialDesc;
	dsMaterialDesc* resolveMaterialDesc;

	dsShaderVariableGroup* transformGroup;
	dsSharedMaterialValues* sharedValues;
	dsMaterial* rMaterial;
	dsMaterial* gMaterial;
	dsMaterial* bMaterial;
	dsMaterial* resolveMaterial;

	dsShader* cubeShader;
	dsShader* resolveShader;
	dsTexture* texture;

	dsOffscreen* rColor;
	dsRenderbuffer* rDepth;
	dsOffscreen* gColor;
	dsRenderbuffer* gDepth;
	dsOffscreen* bColor;
	dsRenderbuffer* bDepth;
	dsRenderbuffer* combinedColor;

	dsGfxBuffer* cubeBuffer;
	dsGfxBuffer* resolveBuffer;
	dsDrawGeometry* cubeGeometry;
	dsDrawGeometry* resolveGeometry;

	uint32_t channelRElement;
	uint32_t channelGElement;
	uint32_t channelBElement;
	uint32_t modelViewProjectionElement;
	float rotation;
	dsMatrix44f view;
	dsMatrix44f projection;
} TestRenderSubpass;

static const char* assetsDir = "TestRenderSubpass-assets";
static char shaderDir[100];

typedef struct Vertex
{
	dsVector3f position;
	dsVector2f texCoord;
} Vertex;

static Vertex vertices[] =
{
	// Front face
	{ {{-1.0f,  1.0f,  1.0f}}, {{0.0f, 0.0f}} },
	{ {{ 1.0f,  1.0f,  1.0f}}, {{1.0f, 0.0f}} },
	{ {{ 1.0f, -1.0f,  1.0f}}, {{1.0f, 1.0f}} },
	{ {{-1.0f, -1.0f,  1.0f}}, {{0.0f, 1.0f}} },

	// Right face
	{ {{ 1.0f,  1.0f,  1.0f}}, {{0.0f, 0.0f}} },
	{ {{ 1.0f,  1.0f, -1.0f}}, {{1.0f, 0.0f}} },
	{ {{ 1.0f, -1.0f, -1.0f}}, {{1.0f, 1.0f}} },
	{ {{ 1.0f, -1.0f,  1.0f}}, {{0.0f, 1.0f}} },

	// Back face
	{ {{ 1.0f,  1.0f, -1.0f}}, {{0.0f, 0.0f}} },
	{ {{-1.0f,  1.0f, -1.0f}}, {{1.0f, 0.0f}} },
	{ {{-1.0f, -1.0f, -1.0f}}, {{1.0f, 1.0f}} },
	{ {{ 1.0f, -1.0f, -1.0f}}, {{0.0f, 1.0f}} },

	// Left face
	{ {{-1.0f,  1.0f, -1.0f}}, {{0.0f, 0.0f}} },
	{ {{-1.0f,  1.0f,  1.0f}}, {{1.0f, 0.0f}} },
	{ {{-1.0f, -1.0f,  1.0f}}, {{1.0f, 1.0f}} },
	{ {{-1.0f, -1.0f, -1.0f}}, {{0.0f, 1.0f}} },

	// Top face
	{ {{-1.0f,  1.0f, -1.0f}}, {{0.0f, 0.0f}} },
	{ {{ 1.0f,  1.0f, -1.0f}}, {{1.0f, 0.0f}} },
	{ {{ 1.0f,  1.0f,  1.0f}}, {{1.0f, 1.0f}} },
	{ {{-1.0f,  1.0f,  1.0f}}, {{0.0f, 1.0f}} },

	// Bottom face
	{ {{-1.0f, -1.0f,  1.0f}}, {{0.0f, 0.0f}} },
	{ {{ 1.0f, -1.0f,  1.0f}}, {{1.0f, 0.0f}} },
	{ {{ 1.0f, -1.0f, -1.0f}}, {{1.0f, 1.0f}} },
	{ {{-1.0f, -1.0f, -1.0f}}, {{0.0f, 1.0f}} },
};

static uint16_t indices[] =
{
	// Front face
	0, 2, 1,
	2, 0, 3,

	// Right face
	4, 6, 5,
	6, 4, 7,

	// Back face
	8, 10, 9,
	10, 8, 11,

	// Left face
	12, 14, 13,
	14, 12, 15,

	// Top face
	16, 18, 17,
	18, 16, 19,

	// Bottom face
	20, 22, 21,
	22, 20, 23,
};

static dsVector2f quad[] =
{
	{{-1.0, -1.0}},
	{{ 1.0, -1.0}},
	{{ 1.0,  1.0}},

	{{ 1.0,  1.0}},
	{{-1.0,  1.0}},
	{{-1.0, -1.0}},
};

static void printHelp(const char* programPath)
{
	printf("usage: %s [OPTIONS]\n", dsPath_getFileName(programPath));
	printf("options:\n");
	printf("  -h, --help      print this help message and exit\n");
	printf("  -r, --renderer <renderer>    explicitly use a renderer; options are:\n");
	for (int i = 0; i < dsRendererType_Default; ++i)
	{
		printf("                                 %s\n",
			dsRenderBootstrap_rendererName((dsRendererType)i));
	}
	printf("  -d, --device <device>        use a graphics device by name\n");
}

static bool validateAllocator(dsAllocator* allocator, const char* name)
{
	if (allocator->size == 0)
		return true;

	DS_LOG_ERROR_F("TestRenderSubpass",
		"Allocator '%s' has %llu bytes allocated with %u allocations.", name,
		(unsigned long long)allocator->size, allocator->currentAllocations);
	return false;
}

static bool createFramebuffer(TestRenderSubpass* testRenderSubpass)
{
	uint32_t width = testRenderSubpass->window->surface->width;
	uint32_t height = testRenderSubpass->window->surface->height;
	uint32_t preRotateWidth = testRenderSubpass->window->surface->preRotateWidth;
	uint32_t preRotateHeight = testRenderSubpass->window->surface->preRotateHeight;

	DS_VERIFY(dsFramebuffer_destroy(testRenderSubpass->framebuffer));
	DS_VERIFY(dsTexture_destroy(testRenderSubpass->rColor));
	DS_VERIFY(dsRenderbuffer_destroy(testRenderSubpass->rDepth));
	DS_VERIFY(dsTexture_destroy(testRenderSubpass->gColor));
	DS_VERIFY(dsRenderbuffer_destroy(testRenderSubpass->gDepth));
	DS_VERIFY(dsTexture_destroy(testRenderSubpass->bColor));
	DS_VERIFY(dsRenderbuffer_destroy(testRenderSubpass->bDepth));
	DS_VERIFY(dsRenderbuffer_destroy(testRenderSubpass->combinedColor));

	dsAllocator* allocator = testRenderSubpass->allocator;
	dsRenderer* renderer = testRenderSubpass->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	dsGfxFormat depthFormat = dsGfxFormat_D24S8;
	if (!dsGfxFormat_renderTargetSupported(resourceManager, depthFormat))
	{
		depthFormat = dsGfxFormat_D32S8_Float;
		if (!dsGfxFormat_renderTargetSupported(resourceManager, depthFormat))
			depthFormat = dsGfxFormat_D16;
	}
	dsGfxFormat colorFormat = dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UNorm);
	dsTextureInfo texInfo = {colorFormat, dsTextureDim_2D, preRotateWidth, preRotateHeight, 0, 1,
		SAMPLE_COUNT};
	dsGfxFormat combinedColorFormat = renderer->surfaceColorFormat;
	dsTextureUsage offscreenUsage = dsTextureUsage_SubpassInput;
	if (!NO_BLIT)
		offscreenUsage |= dsTextureUsage_CopyFrom;
	testRenderSubpass->rColor = dsTexture_createOffscreen(resourceManager, allocator,
		offscreenUsage, dsGfxMemory_Static | dsGfxMemory_GPUOnly, &texInfo, true);
	testRenderSubpass->gColor = dsTexture_createOffscreen(resourceManager, allocator,
		offscreenUsage, dsGfxMemory_Static | dsGfxMemory_GPUOnly, &texInfo, true);
	testRenderSubpass->bColor = dsTexture_createOffscreen(resourceManager, allocator,
		offscreenUsage, dsGfxMemory_Static | dsGfxMemory_GPUOnly, &texInfo, true);
	if (!testRenderSubpass->rColor || !testRenderSubpass->gColor || !testRenderSubpass->bColor)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create offscreen: %s",
			dsErrorString(errno));
		return false;
	}

	testRenderSubpass->rDepth = dsRenderbuffer_create(resourceManager, allocator,
		dsRenderbufferUsage_Standard, depthFormat, preRotateWidth, preRotateHeight, SAMPLE_COUNT);
	testRenderSubpass->gDepth = dsRenderbuffer_create(resourceManager, allocator,
		dsRenderbufferUsage_Standard, depthFormat, preRotateWidth, preRotateHeight, SAMPLE_COUNT);
	testRenderSubpass->bDepth = dsRenderbuffer_create(resourceManager, allocator,
		dsRenderbufferUsage_Standard, depthFormat, preRotateWidth, preRotateHeight, SAMPLE_COUNT);
	if (!testRenderSubpass->rDepth || !testRenderSubpass->gDepth || !testRenderSubpass->bDepth)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create renderbuffer: %s",
			dsErrorString(errno));
		return false;
	}

	// NOTE: Mac seems to have a problem with blitting to the framebuffer.
	if (dsGfxFormat_surfaceBlitSupported(resourceManager, combinedColorFormat,
		testRenderSubpass->renderer->surfaceColorFormat, dsBlitFilter_Linear) && !NO_BLIT)
	{
		testRenderSubpass->combinedColor = dsRenderbuffer_create(resourceManager, allocator,
			dsRenderbufferUsage_BlitFrom, combinedColorFormat, preRotateWidth, preRotateHeight, 1);
		if (!testRenderSubpass->combinedColor)
		{
			DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create renderbuffer: %s",
				dsErrorString(errno));
			return false;
		}
	}

	dsRenderSurface* surface = testRenderSubpass->window->surface;
	dsFramebufferSurface surfaces[] =
	{
		{dsGfxSurfaceType_ColorRenderSurface, dsCubeFace_None, 0, 0, surface},
		{dsGfxSurfaceType_Offscreen, dsCubeFace_None, 0, 0, testRenderSubpass->rColor},
		{dsGfxSurfaceType_Renderbuffer, dsCubeFace_None, 0, 0, testRenderSubpass->rDepth},
		{dsGfxSurfaceType_Offscreen, dsCubeFace_None, 0, 0, testRenderSubpass->gColor},
		{dsGfxSurfaceType_Renderbuffer, dsCubeFace_None, 0, 0, testRenderSubpass->gDepth},
		{dsGfxSurfaceType_Offscreen, dsCubeFace_None, 0, 0, testRenderSubpass->bColor},
		{dsGfxSurfaceType_Renderbuffer, dsCubeFace_None, 0, 0, testRenderSubpass->bDepth}
	};

	if (testRenderSubpass->combinedColor)
	{
		surfaces[0].surfaceType = dsGfxSurfaceType_Renderbuffer;
		surfaces[0].surface = testRenderSubpass->combinedColor;
	}

	uint32_t surfaceCount = DS_ARRAY_SIZE(surfaces);
	testRenderSubpass->framebuffer = dsFramebuffer_create(
		testRenderSubpass->renderer->resourceManager, testRenderSubpass->allocator, "Main",
		surfaces, surfaceCount, preRotateWidth, preRotateHeight, 1);

	if (!testRenderSubpass->framebuffer)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create framebuffer: %s",
			dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsMaterial_setTexture(testRenderSubpass->resolveMaterial,
		testRenderSubpass->channelRElement, testRenderSubpass->rColor));
	DS_VERIFY(dsMaterial_setTexture(testRenderSubpass->resolveMaterial,
		testRenderSubpass->channelGElement, testRenderSubpass->gColor));
	DS_VERIFY(dsMaterial_setTexture(testRenderSubpass->resolveMaterial,
		testRenderSubpass->channelBElement, testRenderSubpass->bColor));

	dsMatrix44f baseProjection, surfaceRotation;
	DS_VERIFY(dsRenderer_makePerspective(&baseProjection, testRenderSubpass->renderer,
		dsDegreesToRadiansf(45.0f), (float)width/(float)height, 0.1f, 100.0f));
	DS_VERIFY(dsRenderSurface_makeRotationMatrix44(&surfaceRotation,
		testRenderSubpass->window->surface->rotation));
	dsMatrix44f_mul(&testRenderSubpass->projection, &surfaceRotation, &baseProjection);

	return true;
}

static bool processEvent(dsApplication* application, dsWindow* window, const dsEvent* event,
	void* userData)
{
	DS_UNUSED(application);

	TestRenderSubpass* testRenderSubpass = (TestRenderSubpass*)userData;
	DS_ASSERT(!window || window == testRenderSubpass->window);
	switch (event->type)
	{
		case dsAppEventType_WindowClosed:
			DS_VERIFY(dsWindow_destroy(window));
			testRenderSubpass->window = NULL;
			return false;
		case dsAppEventType_WindowResized:
		case dsAppEventType_SurfaceInvalidated:
			if (!createFramebuffer(testRenderSubpass))
				abort();
			return true;
		case dsAppEventType_KeyDown:
			if (event->key.key == dsKeyCode_ACBack)
				dsApplication_quit(application, 0);
			return false;
		default:
			return true;
	}
}

static void update(dsApplication* application, float lastFrameTime, void* userData)
{
	DS_UNUSED(application);

	TestRenderSubpass* testRenderSubpass = (TestRenderSubpass*)userData;

	// radians/s
	const float rate = (float)M_PI_2;
	testRenderSubpass->rotation += lastFrameTime*rate;
	while (testRenderSubpass->rotation > 2*M_PI)
		testRenderSubpass->rotation = testRenderSubpass->rotation - (float)(2*M_PI);

	dsMatrix44f model;
	dsMatrix44f_makeRotate(&model, 0, testRenderSubpass->rotation, 0);

	dsMatrix44f modelView, modelViewProjection;
	dsMatrix44f_affineMul(&modelView, &testRenderSubpass->view, &model);
	dsMatrix44f_mul(&modelViewProjection, &testRenderSubpass->projection, &modelView);
	DS_VERIFY(dsShaderVariableGroup_setElementData(testRenderSubpass->transformGroup, 0,
		&modelViewProjection, dsMaterialType_Mat4, 0,  1));
	DS_VERIFY(dsShaderVariableGroup_commit(testRenderSubpass->transformGroup,
		testRenderSubpass->renderer->mainCommandBuffer));
}

static void draw(dsApplication* application, dsWindow* window, void* userData)
{
	DS_UNUSED(application);
	DS_UNUSED(window);
	TestRenderSubpass* testRenderSubpass = (TestRenderSubpass*)userData;
	DS_ASSERT(testRenderSubpass->window == window);
	dsRenderer* renderer = testRenderSubpass->renderer;
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsSurfaceClearValue clearValues[7];
	clearValues[0].colorValue.floatValue.r = 0.0f;
	clearValues[0].colorValue.floatValue.g = 0.0f;
	clearValues[0].colorValue.floatValue.b = 0.0f;
	clearValues[0].colorValue.floatValue.a = 1.0f;
	float channels[3] = {0.1f, 0.2f, 0.4f};
	for (unsigned int i = 0; i < 3; ++i)
	{
		clearValues[i*2 + 1].colorValue.floatValue.r = channels[i];
		clearValues[i*2 + 1].colorValue.floatValue.g = channels[i];
		clearValues[i*2 + 1].colorValue.floatValue.b = channels[i];
		clearValues[i*2 + 1].colorValue.floatValue.a = 1.0f;
		clearValues[i*2 + 2].depthStencil.depth = 1.0f;
		clearValues[i*2 + 2].depthStencil.stencil = 0;
	}
	uint32_t clearValueCount = DS_ARRAY_SIZE(clearValues);
	DS_VERIFY(dsRenderPass_begin(testRenderSubpass->renderPass, commandBuffer,
		testRenderSubpass->framebuffer, NULL, clearValues, clearValueCount, false));

	// Draw red channel
	dsDrawIndexedRange drawRange = {testRenderSubpass->cubeGeometry->indexBuffer.count, 1, 0, 0, 0};
	DS_VERIFY(dsShader_bind(testRenderSubpass->cubeShader, commandBuffer,
		testRenderSubpass->rMaterial, testRenderSubpass->sharedValues, NULL));
	DS_VERIFY(dsRenderer_drawIndexed(renderer, commandBuffer, testRenderSubpass->cubeGeometry,
		&drawRange, dsPrimitiveType_TriangleList));
	DS_VERIFY(dsShader_unbind(testRenderSubpass->cubeShader, commandBuffer));

	DS_VERIFY(dsRenderPass_nextSubpass(testRenderSubpass->renderPass, commandBuffer, false));

	// Draw green channel
	DS_VERIFY(dsShader_bind(testRenderSubpass->cubeShader, commandBuffer,
		testRenderSubpass->gMaterial, testRenderSubpass->sharedValues, NULL));
	DS_VERIFY(dsRenderer_drawIndexed(renderer, commandBuffer, testRenderSubpass->cubeGeometry,
		&drawRange, dsPrimitiveType_TriangleList));
	DS_VERIFY(dsShader_unbind(testRenderSubpass->cubeShader, commandBuffer));

	DS_VERIFY(dsRenderPass_nextSubpass(testRenderSubpass->renderPass, commandBuffer, false));

	// Draw blue channel
	DS_VERIFY(dsShader_bind(testRenderSubpass->cubeShader, commandBuffer,
		testRenderSubpass->bMaterial, testRenderSubpass->sharedValues, NULL));
	DS_VERIFY(dsRenderer_drawIndexed(renderer, commandBuffer, testRenderSubpass->cubeGeometry,
		&drawRange, dsPrimitiveType_TriangleList));
	DS_VERIFY(dsShader_unbind(testRenderSubpass->cubeShader, commandBuffer));

	DS_VERIFY(dsRenderPass_nextSubpass(testRenderSubpass->renderPass, commandBuffer, false));

	// Resolve the final result
	dsDrawRange resolveRange = {testRenderSubpass->resolveGeometry->vertexBuffers[0].count, 1, 0,
		0};
	DS_VERIFY(dsShader_bind(testRenderSubpass->resolveShader, commandBuffer,
		testRenderSubpass->resolveMaterial, NULL, NULL));
	DS_VERIFY(dsRenderer_draw(renderer, commandBuffer, testRenderSubpass->resolveGeometry,
		&resolveRange, dsPrimitiveType_TriangleList));
	DS_VERIFY(dsShader_unbind(testRenderSubpass->resolveShader, commandBuffer));

	DS_VERIFY(dsRenderPass_end(testRenderSubpass->renderPass, commandBuffer));

	// Blit the 3 sub buffers and final buffer to the window if supported.
	if (testRenderSubpass->combinedColor)
	{
		uint32_t width = window->surface->preRotateWidth;
		uint32_t height = window->surface->preRotateHeight;
		dsSurfaceBlitRegion region =
		{
			{dsCubeFace_None, 0, 0, 0, 0},
			{dsCubeFace_None, 0, 0, 0, 0},
			width, height,
			width/2, height/2,
			1
		};
		dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
			testRenderSubpass->rColor, dsGfxSurfaceType_ColorRenderSurface,
			testRenderSubpass->window->surface, &region, 1, dsBlitFilter_Linear);

		region.dstPosition.x = width/2;
		region.dstWidth = width - region.dstPosition.x;
		dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
			testRenderSubpass->gColor, dsGfxSurfaceType_ColorRenderSurface,
			testRenderSubpass->window->surface, &region, 1, dsBlitFilter_Linear);

		region.dstPosition.x = 0;
		region.dstPosition.y = height/2;
		region.dstWidth = width/2;
		region.dstHeight = height - region.dstPosition.y;
		dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
			testRenderSubpass->bColor, dsGfxSurfaceType_ColorRenderSurface,
			testRenderSubpass->window->surface, &region, 1, dsBlitFilter_Linear);

		region.dstPosition.x = width/2;
		region.dstWidth = width - region.dstPosition.x;
		dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Renderbuffer,
			testRenderSubpass->combinedColor, dsGfxSurfaceType_ColorRenderSurface,
			testRenderSubpass->window->surface, &region, 1, dsBlitFilter_Linear);
	}
}

static bool setup(TestRenderSubpass* testRenderSubpass, dsApplication* application,
	dsAllocator* allocator)
{
	dsRenderer* renderer = application->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	testRenderSubpass->allocator = allocator;
	testRenderSubpass->renderer = renderer;

	dsEventResponder responder = {&processEvent, testRenderSubpass, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));
	DS_VERIFY(dsApplication_setUpdateFunction(application, &update, testRenderSubpass, NULL));

	uint32_t width = dsApplication_adjustWindowSize(application, 0, 800);
	uint32_t height = dsApplication_adjustWindowSize(application, 0, 600);
	dsRenderSurfaceUsage surfaceUsage = dsRenderSurfaceUsage_ClientRotations;
	if (!NO_BLIT)
		surfaceUsage |= dsRenderSurfaceUsage_BlitColorTo;
	testRenderSubpass->window = dsWindow_create(application, allocator, "Test Render Subpass",
		NULL, NULL, width, height, dsWindowFlags_Resizeable | dsWindowFlags_DelaySurfaceCreate,
		surfaceUsage);
	if (!testRenderSubpass->window)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create window: %s", dsErrorString(errno));
		return false;
	}

	if (DS_ANDROID || DS_IOS)
		dsWindow_setStyle(testRenderSubpass->window, dsWindowStyle_FullScreen);

	if (!dsWindow_createSurface(testRenderSubpass->window))
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create window surface: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsWindow_setDrawFunction(testRenderSubpass->window, &draw, testRenderSubpass, NULL));

	dsGfxFormat depthFormat = dsGfxFormat_D24S8;
	if (!dsGfxFormat_renderTargetSupported(resourceManager, depthFormat))
	{
		depthFormat = dsGfxFormat_D32S8_Float;
		if (!dsGfxFormat_renderTargetSupported(resourceManager, depthFormat))
		{
			depthFormat = dsGfxFormat_D16;
			if (!dsGfxFormat_renderTargetSupported(resourceManager, depthFormat))
			{
				DS_LOG_ERROR("TestRenderSubpass", "Depth offscreens not supported.");
				return false;
			}
		}
	}

	dsAttachmentInfo attachments[] =
	{
		{dsAttachmentUsage_KeepAfter, renderer->surfaceColorFormat, 1},
		{dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter,
			dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UNorm), SAMPLE_COUNT},
		{dsAttachmentUsage_Clear, depthFormat, SAMPLE_COUNT},
		{dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter,
			dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UNorm), SAMPLE_COUNT},
		{dsAttachmentUsage_Clear, depthFormat, SAMPLE_COUNT},
		{dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter,
			dsGfxFormat_decorate(dsGfxFormat_R8, dsGfxFormat_UNorm), SAMPLE_COUNT},
		{dsAttachmentUsage_Clear, depthFormat, SAMPLE_COUNT}
	};
	uint32_t attachmentCount = DS_ARRAY_SIZE(attachments);

	dsAttachmentRef rColorAttachment = {1, true};
	uint32_t rDepthStencilAttachment = 2;
	dsAttachmentRef gColorAttachment = {3, true};
	uint32_t gDepthStencilAttachment = 4;
	dsAttachmentRef bColorAttachment = {5, true};
	uint32_t bDepthStencilAttachment = 6;
	dsAttachmentRef resolveColorAttachment = {0, false};
	uint32_t inputAttachments[] = {1, 3, 5};

	dsRenderSubpassInfo subpasses[] =
	{
		{"R channel", NULL, &rColorAttachment, {rDepthStencilAttachment, false}, 0, 1},
		{"G channel", NULL, &gColorAttachment, {gDepthStencilAttachment, false}, 0, 1},
		{"B channel", NULL, &bColorAttachment, {bDepthStencilAttachment, false}, 0, 1},
		{"Resolve", inputAttachments, &resolveColorAttachment, {DS_NO_ATTACHMENT, false},
			DS_ARRAY_SIZE(inputAttachments), 1}
	};
	testRenderSubpass->renderPass = dsRenderPass_create(renderer, allocator, attachments,
		attachmentCount, subpasses, DS_ARRAY_SIZE(subpasses), NULL,
		DS_DEFAULT_SUBPASS_DEPENDENCIES);
	if (!testRenderSubpass->renderPass)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create render pass: %s",
			dsErrorString(errno));
		return false;
	}

	char path[DS_PATH_MAX];
	if (!dsPath_combine(path, sizeof(path), assetsDir, shaderDir) ||
		!dsPath_combine(path, sizeof(path), path, "TestRenderSubpass.mslb"))
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create shader path: %s",
			dsErrorString(errno));
		return false;
	}

	testRenderSubpass->shaderModule = dsShaderModule_loadResource(resourceManager, allocator,
		dsFileResourceType_Embedded, path, "TestRenderSubpass");
	if (!testRenderSubpass->shaderModule)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't load shader: %s", dsErrorString(errno));
		return false;
	}

	dsShaderVariableElement groupElems = {"modelViewProjection", dsMaterialType_Mat4, 0};
	testRenderSubpass->transformGroupDesc = dsShaderVariableGroupDesc_create(resourceManager,
		allocator, &groupElems, 1);
	if (!testRenderSubpass->transformGroupDesc)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create shader variable group description: %s",
			dsErrorString(errno));
		return false;
	}

	{
		dsMaterialElement materialElems[] =
		{
			{"Transform", dsMaterialType_VariableGroup, 0, testRenderSubpass->transformGroupDesc,
				dsMaterialBinding_Global, 0},
			{"channel", dsMaterialType_Int, 0, NULL, dsMaterialBinding_Material, 0},
			{"tex", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0}
		};
		testRenderSubpass->cubeMaterialDesc = dsMaterialDesc_create(resourceManager, allocator,
			materialElems, DS_ARRAY_SIZE(materialElems));
		if (!testRenderSubpass->cubeMaterialDesc)
		{
			DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create material description: %s",
				dsErrorString(errno));
			return false;
		}
	}

	{
		dsMaterialElement materialElems[] =
		{
			{"channelR", dsMaterialType_SubpassInput, 0, NULL, dsMaterialBinding_Material, 0},
			{"channelG", dsMaterialType_SubpassInput, 0, NULL, dsMaterialBinding_Material, 0},
			{"channelB", dsMaterialType_SubpassInput, 0, NULL, dsMaterialBinding_Material, 0}
		};
		testRenderSubpass->resolveMaterialDesc = dsMaterialDesc_create(resourceManager, allocator,
			materialElems, DS_ARRAY_SIZE(materialElems));
		if (!testRenderSubpass->resolveMaterialDesc)
		{
			DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create material description: %s",
				dsErrorString(errno));
			return false;
		}
	}

	testRenderSubpass->transformGroup = dsShaderVariableGroup_create(resourceManager, allocator,
		allocator, testRenderSubpass->transformGroupDesc);
	if (!testRenderSubpass->transformGroup)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create shader variable group: %s",
			dsErrorString(errno));
		return false;
	}

	testRenderSubpass->sharedValues = dsSharedMaterialValues_create(allocator,
		DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	if (!testRenderSubpass->sharedValues)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create shared material values: %s",
			dsErrorString(errno));
		return false;
	}
	DS_VERIFY(dsSharedMaterialValues_setVariableGroupName(testRenderSubpass->sharedValues,
		"Transform", testRenderSubpass->transformGroup));

	testRenderSubpass->rMaterial = dsMaterial_create(resourceManager, allocator,
		testRenderSubpass->cubeMaterialDesc);
	testRenderSubpass->gMaterial = dsMaterial_create(resourceManager, allocator,
		testRenderSubpass->cubeMaterialDesc);
	testRenderSubpass->bMaterial = dsMaterial_create(resourceManager, allocator,
		testRenderSubpass->cubeMaterialDesc);
	testRenderSubpass->resolveMaterial = dsMaterial_create(resourceManager, allocator,
		testRenderSubpass->resolveMaterialDesc);
	if (!testRenderSubpass->rMaterial || !testRenderSubpass->gMaterial ||
		!testRenderSubpass->bMaterial || !testRenderSubpass->resolveMaterial)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create material: %s", dsErrorString(errno));
		return false;
	}

	testRenderSubpass->cubeShader = dsShader_createName(resourceManager, allocator,
		testRenderSubpass->shaderModule, "Cube", testRenderSubpass->cubeMaterialDesc);
	if (!testRenderSubpass->cubeShader)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create shader: %s", dsErrorString(errno));
		return false;
	}

	testRenderSubpass->resolveShader = dsShader_createName(resourceManager, allocator,
		testRenderSubpass->shaderModule, "Resolve", testRenderSubpass->resolveMaterialDesc);
	if (!testRenderSubpass->resolveShader)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create shader: %s", dsErrorString(errno));
		return false;
	}

	if (!dsPath_combine(path, sizeof(path), assetsDir, "texture.pvr"))
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create texture path: %s",
			dsErrorString(errno));
		return false;
	}

	testRenderSubpass->texture = dsTextureData_loadResourceToTexture(resourceManager, allocator,
		NULL, dsFileResourceType_Embedded, path, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static | dsGfxMemory_GPUOnly);
	if (!testRenderSubpass->texture)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't load texture: %s", dsErrorString(errno));
		return false;
	}

	uint32_t texElement = dsMaterialDesc_findElement(testRenderSubpass->cubeMaterialDesc, "tex");
	DS_ASSERT(texElement != DS_MATERIAL_UNKNOWN);
	uint32_t channelElement = dsMaterialDesc_findElement(testRenderSubpass->cubeMaterialDesc,
		"channel");
	DS_ASSERT(channelElement != DS_MATERIAL_UNKNOWN);

	DS_VERIFY(dsMaterial_setTexture(testRenderSubpass->rMaterial, texElement,
		testRenderSubpass->texture));
	int channel = 0;
	DS_VERIFY(dsMaterial_setElementData(testRenderSubpass->rMaterial, channelElement,
		&channel, dsMaterialType_Int, 0, 1));

	DS_VERIFY(dsMaterial_setTexture(testRenderSubpass->gMaterial, texElement,
		testRenderSubpass->texture));
	channel = 1;
	DS_VERIFY(dsMaterial_setElementData(testRenderSubpass->gMaterial, channelElement,
		&channel, dsMaterialType_Int, 0, 1));

	DS_VERIFY(dsMaterial_setTexture(testRenderSubpass->bMaterial, texElement,
		testRenderSubpass->texture));
	channel = 2;
	DS_VERIFY(dsMaterial_setElementData(testRenderSubpass->bMaterial, channelElement,
		&channel, dsMaterialType_Int, 0, 1));

	uint8_t combinedBufferData[sizeof(vertices) + sizeof(indices)];
	memcpy(combinedBufferData, vertices, sizeof(vertices));
	memcpy(combinedBufferData + sizeof(vertices), indices, sizeof(indices));
	testRenderSubpass->cubeBuffer = dsGfxBuffer_create(resourceManager, allocator,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_Index,
		dsGfxMemory_Static | dsGfxMemory_Draw | dsGfxMemory_GPUOnly, combinedBufferData,
		sizeof(combinedBufferData));
	if (!testRenderSubpass->cubeBuffer)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create graphics buffer: %s",
			dsErrorString(errno));
		return false;
	}

	testRenderSubpass->resolveBuffer = dsGfxBuffer_create(resourceManager, allocator,
		dsGfxBufferUsage_Vertex, dsGfxMemory_Static | dsGfxMemory_Draw | dsGfxMemory_GPUOnly, quad,
		sizeof(quad));
	if (!testRenderSubpass->resolveBuffer)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create graphics buffer: %s",
			dsErrorString(errno));
		return false;
	}

	{
		dsVertexFormat vertexFormat;
		DS_VERIFY(dsVertexFormat_initialize(&vertexFormat));
		vertexFormat.elements[dsVertexAttrib_Position].format =
			dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
		DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position, true));
		vertexFormat.elements[dsVertexAttrib_TexCoord0].format =
			dsGfxFormat_decorate(dsGfxFormat_X32Y32, dsGfxFormat_Float);
		DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord0, true));
		DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));
		DS_ASSERT(vertexFormat.size == sizeof(Vertex));
		DS_ASSERT(vertexFormat.elements[dsVertexAttrib_Position].offset ==
			offsetof(Vertex, position));
		DS_ASSERT(vertexFormat.elements[dsVertexAttrib_TexCoord0].offset ==
			offsetof(Vertex, texCoord));

		dsVertexBuffer vertexBuffer = {testRenderSubpass->cubeBuffer, 0,
			DS_ARRAY_SIZE(vertices), vertexFormat};
		dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {&vertexBuffer, NULL, NULL,
			NULL};
		dsIndexBuffer indexBuffer = {testRenderSubpass->cubeBuffer, sizeof(vertices),
			DS_ARRAY_SIZE(indices), (uint32_t)sizeof(uint16_t)};
		testRenderSubpass->cubeGeometry = dsDrawGeometry_create(resourceManager, allocator,
			vertexBuffers, &indexBuffer);
		if (!testRenderSubpass->cubeGeometry)
		{
			DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create geometry: %s",
				dsErrorString(errno));
			return false;
		}
	}

	{
		dsVertexFormat vertexFormat;
		DS_VERIFY(dsVertexFormat_initialize(&vertexFormat));
		vertexFormat.elements[dsVertexAttrib_Position].format =
			dsGfxFormat_decorate(dsGfxFormat_X32Y32, dsGfxFormat_Float);
		DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position, true));
		DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));
		DS_ASSERT(vertexFormat.size == sizeof(dsVector2f));

		dsVertexBuffer vertexBuffer = {testRenderSubpass->resolveBuffer, 0,
			DS_ARRAY_SIZE(quad), vertexFormat};
		dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {&vertexBuffer, NULL, NULL,
			NULL};
		testRenderSubpass->resolveGeometry = dsDrawGeometry_create(resourceManager, allocator,
			vertexBuffers, NULL);
		if (!testRenderSubpass->resolveGeometry)
		{
			DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create geometry: %s",
				dsErrorString(errno));
			return false;
		}
	}

	testRenderSubpass->channelRElement = dsMaterialDesc_findElement(
		testRenderSubpass->resolveMaterialDesc, "channelR");
	DS_ASSERT(testRenderSubpass->channelRElement != DS_MATERIAL_UNKNOWN);
	testRenderSubpass->channelGElement = dsMaterialDesc_findElement(
		testRenderSubpass->resolveMaterialDesc, "channelG");
	DS_ASSERT(testRenderSubpass->channelGElement != DS_MATERIAL_UNKNOWN);
	testRenderSubpass->channelBElement = dsMaterialDesc_findElement(
		testRenderSubpass->resolveMaterialDesc, "channelB");
	DS_ASSERT(testRenderSubpass->channelBElement != DS_MATERIAL_UNKNOWN);
	testRenderSubpass->modelViewProjectionElement = dsShaderVariableGroupDesc_findElement(
		testRenderSubpass->transformGroupDesc, "modelViewProjection");
	DS_ASSERT(testRenderSubpass->modelViewProjectionElement != DS_MATERIAL_UNKNOWN);
	testRenderSubpass->rotation = 0;
	dsVector3f eyePos = {{0.0f, 5.0f, 5.0f}};
	dsVector3f lookAtPos = {{0.0f, 0.0f, 0.0f}};
	dsVector3f upDir = {{0.0f, 1.0f, 0.0f}};
	dsMatrix44f camera;
	dsMatrix44f_lookAt(&camera, &eyePos, &lookAtPos, &upDir);
	dsMatrix44f_affineInvert(&testRenderSubpass->view, &camera);

	if (!createFramebuffer(testRenderSubpass))
		return false;

	return true;
}

static void shutdown(TestRenderSubpass* testRenderSubpass)
{
	DS_VERIFY(dsDrawGeometry_destroy(testRenderSubpass->resolveGeometry));
	DS_VERIFY(dsDrawGeometry_destroy(testRenderSubpass->cubeGeometry));
	DS_VERIFY(dsGfxBuffer_destroy(testRenderSubpass->resolveBuffer));
	DS_VERIFY(dsGfxBuffer_destroy(testRenderSubpass->cubeBuffer));
	DS_VERIFY(dsRenderbuffer_destroy(testRenderSubpass->combinedColor));
	DS_VERIFY(dsRenderbuffer_destroy(testRenderSubpass->bDepth));
	DS_VERIFY(dsTexture_destroy(testRenderSubpass->bColor));
	DS_VERIFY(dsRenderbuffer_destroy(testRenderSubpass->gDepth));
	DS_VERIFY(dsTexture_destroy(testRenderSubpass->gColor));
	DS_VERIFY(dsRenderbuffer_destroy(testRenderSubpass->rDepth));
	DS_VERIFY(dsTexture_destroy(testRenderSubpass->rColor));
	DS_VERIFY(dsTexture_destroy(testRenderSubpass->texture));
	DS_VERIFY(dsShader_destroy(testRenderSubpass->resolveShader));
	DS_VERIFY(dsShader_destroy(testRenderSubpass->cubeShader));
	dsMaterial_destroy(testRenderSubpass->resolveMaterial);
	dsMaterial_destroy(testRenderSubpass->bMaterial);
	dsMaterial_destroy(testRenderSubpass->gMaterial);
	dsMaterial_destroy(testRenderSubpass->rMaterial);
	dsSharedMaterialValues_destroy(testRenderSubpass->sharedValues);
	DS_VERIFY(dsShaderVariableGroup_destroy(testRenderSubpass->transformGroup));
	DS_VERIFY(dsMaterialDesc_destroy(testRenderSubpass->resolveMaterialDesc));
	DS_VERIFY(dsShaderVariableGroupDesc_destroy(testRenderSubpass->transformGroupDesc));
	DS_VERIFY(dsMaterialDesc_destroy(testRenderSubpass->cubeMaterialDesc));
	DS_VERIFY(dsShaderModule_destroy(testRenderSubpass->shaderModule));
	DS_VERIFY(dsRenderPass_destroy(testRenderSubpass->renderPass));
	DS_VERIFY(dsFramebuffer_destroy(testRenderSubpass->framebuffer));
	DS_VERIFY(dsWindow_destroy(testRenderSubpass->window));
}

int dsMain(int argc, const char** argv)
{
	dsRendererType rendererType = dsRendererType_Default;
	const char* deviceName = NULL;
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			printHelp(argv[0]);
			return 0;
		}
		else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--renderer") == 0)
		{
			if (i == argc - 1)
			{
				printf("--renderer option requires an argument\n");
				printHelp(argv[0]);
				return 1;
			}
			rendererType = dsRenderBootstrap_rendererTypeFromName(argv[++i]);
			if (rendererType == dsRendererType_Default)
			{
				printf("Unknown renderer type: %s\n", argv[i]);
				printHelp(argv[0]);
				return 1;
			}
		}
		else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--device") == 0)
		{
			if (i == argc - 1)
			{
				printf("--device option requires an argument\n");
				printHelp(argv[0]);
				return 1;
			}
			deviceName = argv[++i];
		}
		else if (*argv[i])
		{
			printf("Unknown option: %s\n", argv[i]);
			printHelp(argv[0]);
			return 1;
		}
	}

	DS_LOG_INFO_F("TestRenderSubpass", "Render using %s",
		dsRenderBootstrap_rendererName(rendererType));

	dsSystemAllocator renderAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&renderAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator applicationAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&applicationAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator testRenderSubpassAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&testRenderSubpassAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRendererOptions rendererOptions;
	dsRenderer_defaultOptions(&rendererOptions, "TestRenderSubpass", 0);
	rendererOptions.deviceName = deviceName;
	rendererOptions.alphaBits = 8;
	rendererOptions.depthBits = 0;
	rendererOptions.stencilBits = 0;
	dsRenderer* renderer = dsRenderBootstrap_createRenderer(rendererType,
		(dsAllocator*)&renderAllocator, &rendererOptions);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create renderer: %s", dsErrorString(errno));
		return 2;
	}

	dsRenderer_setVSync(renderer, dsVSync_TripleBuffer);
#if DS_DEBUG
	dsRenderer_setExtraDebugging(renderer, true);
#endif

	dsShaderVersion shaderVersions[] =
	{
		{DS_VK_RENDERER_ID, DS_ENCODE_VERSION(1, 0, 0)},
		{DS_MTL_RENDERER_ID, DS_ENCODE_VERSION(1, 1, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(1, 1, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(1, 5, 0)},
		{DS_GLES_RENDERER_ID, DS_ENCODE_VERSION(1, 0, 0)},
		{DS_GLES_RENDERER_ID, DS_ENCODE_VERSION(3, 0, 0)}
	};
	DS_VERIFY(dsRenderer_shaderVersionToString(shaderDir, DS_ARRAY_SIZE(shaderDir), renderer,
		dsRenderer_chooseShaderVersion(renderer, shaderVersions, DS_ARRAY_SIZE(shaderVersions))));

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer, argc, argv, "DeepSea", "TestRenderSubpass", dsSDLApplicationFlags_None);
	if (!application)
	{
		DS_LOG_ERROR_F("TestRenderSubpass", "Couldn't create application: %s", dsErrorString(errno));
		dsRenderer_destroy(renderer);
		return 2;
	}

	TestRenderSubpass testRenderSubpass;
	memset(&testRenderSubpass, 0, sizeof(testRenderSubpass));
	if (!setup(&testRenderSubpass, application, (dsAllocator*)&testRenderSubpassAllocator))
	{
		shutdown(&testRenderSubpass);
		return 3;
	}

	int exitCode = dsApplication_run(application);

	shutdown(&testRenderSubpass);
	dsSDLApplication_destroy(application);
	dsRenderer_destroy(renderer);

	if (!validateAllocator((dsAllocator*)&renderAllocator, "render"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&applicationAllocator, "application"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&testRenderSubpassAllocator, "TestRenderSubpass"))
		exitCode = 4;

	return exitCode;
}
