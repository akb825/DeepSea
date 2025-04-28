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
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/TextureData.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Render/RenderSurface.h>

#include <DeepSea/RenderBootstrap/RenderBootstrap.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TestCube
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsWindow* window;
	dsFramebuffer* framebuffer;
	dsRenderPass* renderPass;
	dsShaderModule* shaderModule;
	dsMaterialDesc* materialDesc;
	dsMaterial* material;
	dsShader* shader;
	dsTexture* texture;
	dsGfxBuffer* drawBuffer;
	dsDrawGeometry* geometry;

	uint64_t invalidatedFrame;
	uint32_t modelViewProjectionElement;
	float rotation;
	dsMatrix44f view;
	dsMatrix44f projection;
} TestCube;

static const char* assetsDir = "TestCube-assets";
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

static void printHelp(const char* programPath)
{
	printf("usage: %s [OPTIONS]\n", dsPath_getFileName(programPath));
	printf("options:\n");
	printf("  -h, --help                   print this help message and exit\n");
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

	DS_LOG_ERROR_F("TestCube", "Allocator '%s' has %llu bytes allocated with %u allocations.",
		name, (unsigned long long)allocator->size, allocator->currentAllocations);
	return false;
}

static bool createFramebuffer(TestCube* testCube)
{
	uint32_t width = testCube->window->surface->width;
	uint32_t height = testCube->window->surface->height;
	uint32_t preRotateWidth = testCube->window->surface->preRotateWidth;
	uint32_t preRotateHeight = testCube->window->surface->preRotateHeight;

	dsFramebuffer_destroy(testCube->framebuffer);

	dsRenderSurface* surface = testCube->window->surface;
	dsFramebufferSurface surfaces[] =
	{
		{dsGfxSurfaceType_ColorRenderSurface, dsCubeFace_None, 0, 0, surface},
		{dsGfxSurfaceType_DepthRenderSurface, dsCubeFace_None, 0, 0, surface}
	};
	testCube->framebuffer = dsFramebuffer_create(testCube->renderer->resourceManager,
		testCube->allocator, "Main", surfaces, 2, preRotateWidth, preRotateHeight, 1);

	if (!testCube->framebuffer)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create framebuffer: %s", dsErrorString(errno));
		return false;
	}

	dsMatrix44f baseProjection, surfaceRotation;
	DS_VERIFY(dsRenderer_makePerspective(&baseProjection, testCube->renderer,
		dsDegreesToRadiansf(45.0f), (float)width/(float)height, 0.1f, 100.0f));
	DS_VERIFY(dsRenderSurface_makeRotationMatrix44(&surfaceRotation,
		testCube->window->surface->rotation));
	dsMatrix44f_mul(&testCube->projection, &surfaceRotation, &baseProjection);

	return true;
}

static bool processEvent(dsApplication* application, dsWindow* window, const dsEvent* event,
	void* userData)
{
	TestCube* testCube = (TestCube*)userData;
	dsRenderer* renderer = application->renderer;
	DS_ASSERT(!window || window == testCube->window);
	switch (event->type)
	{
		case dsAppEventType_WindowClosed:
			DS_VERIFY(dsWindow_destroy(window));
			testCube->window = NULL;
			return false;
		case dsAppEventType_WindowResized:
		case dsAppEventType_SurfaceInvalidated:
			if (!createFramebuffer(testCube))
				abort();
			testCube->invalidatedFrame = renderer->frameNumber;
			return true;
		case dsAppEventType_KeyDown:
			if (event->key.repeat)
				return false;

			if (event->key.key == dsKeyCode_ACBack)
				dsApplication_quit(application, 0);
			else if (event->key.key == dsKeyCode_1)
			{
				// The key down will be re-sent when re-creating the window.
				if (testCube->invalidatedFrame + 2 > renderer->frameNumber)
					return false;

				uint32_t samples = renderer->surfaceSamples;
				if (samples == 1)
					samples = 4;
				else
					samples = 1;
				dsRenderer_setSamples(renderer, samples);
			}
			else if (event->key.key == dsKeyCode_2)
			{
				if (renderer->vsync == dsVSync_Disabled)
					dsRenderer_setVSync(renderer, dsVSync_TripleBuffer);
				else
					dsRenderer_setVSync(renderer, dsVSync_Disabled);
			}
			else if (event->key.key == dsKeyCode_3)
			{
				float anisotropy = renderer->defaultAnisotropy;
				if (anisotropy == 1.0f)
					anisotropy = renderer->maxAnisotropy;
				else
					anisotropy = 1.0f;
				dsRenderer_setDefaultAnisotropy(renderer, anisotropy);
			}
			return false;
		default:
			return true;
	}
}

static void update(dsApplication* application, float lastFrameTime, void* userData)
{
	DS_UNUSED(application);

	TestCube* testCube = (TestCube*)userData;

	// radians/s
	const float rate = M_PI_2f;
	testCube->rotation += lastFrameTime*rate;
	while (testCube->rotation > 2*M_PIf)
		testCube->rotation = testCube->rotation - 2*M_PIf;

	dsMatrix44f model;
	dsMatrix44f_makeRotate(&model, 0, testCube->rotation, 0);

	dsMatrix44f modelView, modelViewProjection;
	dsMatrix44f_affineMul(&modelView, &testCube->view, &model);
	dsMatrix44f_mul(&modelViewProjection, &testCube->projection, &modelView);
	DS_VERIFY(dsMaterial_setElementData(testCube->material, 0, &modelViewProjection,
		dsMaterialType_Mat4, 0, 1));
}

static void draw(dsApplication* application, dsWindow* window, void* userData)
{
	DS_UNUSED(application);
	DS_UNUSED(window);
	TestCube* testCube = (TestCube*)userData;
	DS_ASSERT(testCube->window == window);
	dsRenderer* renderer = testCube->renderer;
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsSurfaceClearValue clearValues[2];
	clearValues[0].colorValue.floatValue.r = 0.1f;
	clearValues[0].colorValue.floatValue.g = 0.2f;
	clearValues[0].colorValue.floatValue.b = 0.4f;
	clearValues[0].colorValue.floatValue.a = 1.0f;
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;
	DS_VERIFY(dsRenderPass_begin(testCube->renderPass, commandBuffer, testCube->framebuffer, NULL,
		clearValues, 2, false));
	DS_VERIFY(dsShader_bind(testCube->shader, commandBuffer, testCube->material, NULL, NULL));

	dsDrawIndexedRange drawRange = {testCube->geometry->indexBuffer.count, 1, 0, 0, 0};
	DS_VERIFY(dsRenderer_drawIndexed(renderer, commandBuffer, testCube->geometry, &drawRange,
		dsPrimitiveType_TriangleList));

	DS_VERIFY(dsShader_unbind(testCube->shader, commandBuffer));
	DS_VERIFY(dsRenderPass_end(testCube->renderPass, commandBuffer));
}

static bool setup(TestCube* testCube, dsApplication* application, dsAllocator* allocator)
{
	dsRenderer* renderer = application->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	testCube->allocator = allocator;
	testCube->renderer = renderer;

	dsEventResponder responder = {&processEvent, testCube, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));
	DS_VERIFY(dsApplication_setUpdateFunction(application, &update, testCube, NULL));

	uint32_t width = dsApplication_adjustWindowSize(application, 0, 800);
	uint32_t height = dsApplication_adjustWindowSize(application, 0, 600);
	testCube->window = dsWindow_create(application, allocator, "Test Cube", NULL,
		NULL, width, height, dsWindowFlags_Resizeable | dsWindowFlags_DelaySurfaceCreate,
		dsRenderSurfaceUsage_ClientRotations);
	if (!testCube->window)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create window: %s", dsErrorString(errno));
		return false;
	}

	if (DS_ANDROID || DS_IOS)
		dsWindow_setStyle(testCube->window, dsWindowStyle_FullScreen);

	if (!dsWindow_createSurface(testCube->window))
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create window surface: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsWindow_setDrawFunction(testCube->window, &draw, testCube, NULL));

	if (!createFramebuffer(testCube))
		return false;

	dsAttachmentInfo attachments[] =
	{
		{dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter, renderer->surfaceColorFormat,
			DS_SURFACE_ANTIALIAS_SAMPLES},
		{dsAttachmentUsage_Clear, renderer->surfaceDepthStencilFormat, DS_SURFACE_ANTIALIAS_SAMPLES}
	};

	dsAttachmentRef colorAttachment = {0, true};
	uint32_t depthStencilAttachment = 1;
	dsRenderSubpassInfo subpass =
	{
		"TestCube", NULL, &colorAttachment, {depthStencilAttachment, false}, 0, 1
	};
	testCube->renderPass = dsRenderPass_create(renderer, allocator, attachments, 2, &subpass, 1,
		NULL, DS_DEFAULT_SUBPASS_DEPENDENCIES);
	if (!testCube->renderPass)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create render pass: %s", dsErrorString(errno));
		return false;
	}

	char path[DS_PATH_MAX];
	if (!dsPath_combine(path, sizeof(path), assetsDir, shaderDir) ||
		!dsPath_combine(path, sizeof(path), path, "TestCube.mslb"))
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create shader path: %s", dsErrorString(errno));
		return false;
	}

	testCube->shaderModule = dsShaderModule_loadResource(resourceManager, allocator,
		dsFileResourceType_Embedded, path, "TestCube");
	if (!testCube->shaderModule)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't load shader: %s", dsErrorString(errno));
		return false;
	}

	dsMaterialElement materialElems[] =
	{
		{"modelViewProjection", dsMaterialType_Mat4, 0, NULL, dsMaterialBinding_Material, 0},
		{"tex", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0}
	};
	testCube->materialDesc = dsMaterialDesc_create(resourceManager, allocator, materialElems,
		DS_ARRAY_SIZE(materialElems));
	if (!testCube->materialDesc)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create material description: %s",
			dsErrorString(errno));
		return false;
	}

	testCube->material = dsMaterial_create(resourceManager, allocator, testCube->materialDesc);
	if (!testCube->material)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create material: %s", dsErrorString(errno));
		return false;
	}

	testCube->shader = dsShader_createName(resourceManager, allocator, testCube->shaderModule,
		"Default", testCube->materialDesc);
	if (!testCube->shader)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create shader: %s", dsErrorString(errno));
		return false;
	}

	if (!dsPath_combine(path, sizeof(path), assetsDir, "texture.pvr"))
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create texture path: %s", dsErrorString(errno));
		return false;
	}

	testCube->texture = dsTextureData_loadResourceToTexture(resourceManager, allocator, NULL,
		dsFileResourceType_Embedded, path, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static | dsGfxMemory_GPUOnly);
	if (!testCube->texture)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't load texture: %s", dsErrorString(errno));
		return false;
	}

	uint32_t texElement = dsMaterialDesc_findElement(testCube->materialDesc, "tex");
	DS_ASSERT(texElement != DS_MATERIAL_UNKNOWN);
	DS_VERIFY(dsMaterial_setTexture(testCube->material, texElement, testCube->texture));

	uint8_t combinedBufferData[sizeof(vertices) + sizeof(indices)];
	memcpy(combinedBufferData, vertices, sizeof(vertices));
	memcpy(combinedBufferData + sizeof(vertices), indices, sizeof(indices));
	testCube->drawBuffer = dsGfxBuffer_create(resourceManager, allocator,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_Index,
		dsGfxMemory_Static | dsGfxMemory_Draw | dsGfxMemory_GPUOnly, combinedBufferData,
		sizeof(combinedBufferData));
	if (!testCube->drawBuffer)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create graphics buffer: %s", dsErrorString(errno));
		return false;
	}

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
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_Position].offset == offsetof(Vertex, position));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_TexCoord0].offset == offsetof(Vertex, texCoord));

	dsVertexBuffer vertexBuffer = {testCube->drawBuffer, 0, DS_ARRAY_SIZE(vertices), vertexFormat};
	dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {&vertexBuffer, NULL, NULL,
		NULL};
	dsIndexBuffer indexBuffer = {testCube->drawBuffer, sizeof(vertices),
		DS_ARRAY_SIZE(indices), (uint32_t)sizeof(uint16_t)};
	testCube->geometry = dsDrawGeometry_create(resourceManager, allocator, vertexBuffers,
		&indexBuffer);
	if (!testCube->geometry)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create geometry: %s", dsErrorString(errno));
		return false;
	}

	testCube->modelViewProjectionElement = dsMaterialDesc_findElement(testCube->materialDesc,
		"modelViewProjection");
	DS_ASSERT(testCube->modelViewProjectionElement != DS_MATERIAL_UNKNOWN);
	testCube->rotation = 0;
	dsVector3f eyePos = {{0.0f, 5.0f, 5.0f}};
	dsVector3f lookAtPos = {{0.0f, 0.0f, 0.0f}};
	dsVector3f upDir = {{0.0f, 1.0f, 0.0f}};
	dsMatrix44f camera;
	dsMatrix44f_lookAt(&camera, &eyePos, &lookAtPos, &upDir);
	dsMatrix44f_affineInvert(&testCube->view, &camera);

	return true;
}

static void shutdown(TestCube* testCube)
{
	DS_VERIFY(dsDrawGeometry_destroy(testCube->geometry));
	DS_VERIFY(dsGfxBuffer_destroy(testCube->drawBuffer));
	DS_VERIFY(dsTexture_destroy(testCube->texture));
	DS_VERIFY(dsShader_destroy(testCube->shader));
	dsMaterial_destroy(testCube->material);
	DS_VERIFY(dsMaterialDesc_destroy(testCube->materialDesc));
	DS_VERIFY(dsShaderModule_destroy(testCube->shaderModule));
	DS_VERIFY(dsRenderPass_destroy(testCube->renderPass));
	DS_VERIFY(dsFramebuffer_destroy(testCube->framebuffer));
	DS_VERIFY(dsWindow_destroy(testCube->window));
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

	DS_LOG_INFO_F("TestCube", "Render using %s", dsRenderBootstrap_rendererName(rendererType));
	DS_LOG_INFO("TestCube", "Press '1' to toggle anti-aliasing.");
	DS_LOG_INFO("TestCube", "Press '2' to toggle vsync.");
	DS_LOG_INFO("TestCube", "Press '3' to toggle anisotropic filtering.");

	dsSystemAllocator renderAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&renderAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator applicationAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&applicationAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator testCubeAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&testCubeAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRendererOptions rendererOptions;
	dsRenderer_defaultOptions(&rendererOptions, "TestCube", 0);
	rendererOptions.surfaceSamples = 4;
	rendererOptions.deviceName = deviceName;
	if (!dsSDLApplication_prepareRendererOptions(
			&rendererOptions, dsRenderBootstrap_rendererID(rendererType)))
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't setup renderer options.");
		return 0;
	}

	dsRenderer* renderer = dsRenderBootstrap_createRenderer(rendererType,
		(dsAllocator*)&renderAllocator, &rendererOptions);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create renderer: %s", dsErrorString(errno));
		return 2;
	}

	dsRenderer_setVSync(renderer, dsVSync_TripleBuffer);
	dsRenderer_setDefaultAnisotropy(renderer, renderer->maxAnisotropy);
#if DS_DEBUG
	dsRenderer_setExtraDebugging(renderer, true);
#endif

	dsShaderVersion shaderVersions[] =
	{
		{DS_VK_RENDERER_ID, DS_ENCODE_VERSION(1, 0, 0)},
		{DS_MTL_RENDERER_ID, DS_ENCODE_VERSION(1, 1, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(1, 1, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(1, 5, 0)},
		{DS_GLES_RENDERER_ID, DS_ENCODE_VERSION(1, 0, 0)}
	};
	DS_VERIFY(dsRenderer_shaderVersionToString(shaderDir, DS_ARRAY_SIZE(shaderDir), renderer,
		dsRenderer_chooseShaderVersion(renderer, shaderVersions, DS_ARRAY_SIZE(shaderVersions))));

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer, argc, argv, "DeepSea", "TestCube", dsSDLApplicationFlags_None);
	if (!application)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create application: %s", dsErrorString(errno));
		dsRenderer_destroy(renderer);
		return 2;
	}

	TestCube testCube;
	memset(&testCube, 0, sizeof(testCube));
	if (!setup(&testCube, application, (dsAllocator*)&testCubeAllocator))
	{
		shutdown(&testCube);
		return 3;
	}

	int exitCode = dsApplication_run(application);

	shutdown(&testCube);
	dsSDLApplication_destroy(application);
	dsRenderer_destroy(renderer);

	if (!validateAllocator((dsAllocator*)&renderAllocator, "render"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&applicationAllocator, "application"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&testCubeAllocator, "TestCube"))
		exitCode = 4;

	return exitCode;
}
