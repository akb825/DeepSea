/*
 * Copyright 2019 Aaron Barany
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

#include "FixtureBase.h"
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/RenderBootstrap/RenderBootstrap.h>

static std::vector<dsRendererType> getSupportedRenderers()
{
	std::vector<dsRendererType> renderers;
	for (int i = 0; i < dsRendererType_Default; ++i)
	{
		if (dsRenderBootstrap_isSupported((dsRendererType)i))
			renderers.push_back((dsRendererType)i);
	}
	return renderers;
}

testing::internal::ParamGenerator<dsRendererType> FixtureBase::getRendererTypes()
{
	static std::vector<dsRendererType> rendererTypes = getSupportedRenderers();
	return testing::ValuesIn(rendererTypes);
}

void FixtureBase::SetUp()
{
	dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT);
	dsRendererOptions options;
	dsRenderer_defaultOptions(&options, "deepsea_test_render_bootstrap", 0);
	options.samples = 0;
	renderer = dsRenderBootstrap_createRenderer(GetParam(), &allocator.allocator,
		&options);
	ASSERT_TRUE(renderer);
	resourceManager = renderer->resourceManager;

	dsShaderVersion shaderVersions[] =
	{
		{DS_VK_RENDERER_ID, DS_ENCODE_VERSION(1, 0, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(1, 1, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(4, 1, 0)},
		{DS_GLES_RENDERER_ID, DS_ENCODE_VERSION(1, 0, 0)},
		{DS_GLES_RENDERER_ID, DS_ENCODE_VERSION(3, 1, 0)},
	};
	const dsShaderVersion* shaderVersion = dsRenderer_chooseShaderVersion(renderer, shaderVersions,
		DS_ARRAY_SIZE(shaderVersions));
	ASSERT_TRUE(shaderVersion);

	ASSERT_TRUE(dsRenderer_shaderVersionToString(m_buffer, sizeof(m_buffer), renderer,
		shaderVersion));
	ASSERT_TRUE(dsPath_combine(m_shaderDir, sizeof(m_shaderDir), "RenderBootstrapTest-assets",
		m_buffer));

	EXPECT_TRUE(dsRenderer_beginFrame(renderer));
}

void FixtureBase::TearDown()
{
	EXPECT_TRUE(dsRenderer_endFrame(renderer));

	dsRenderer_destroy(renderer);
	EXPECT_EQ(0U, allocator.allocator.size);
}

const char* FixtureBase::getShaderPath(const char* fileName) const
{
	dsPath_combine(m_buffer, sizeof(m_buffer), m_shaderDir, fileName);
	return m_buffer;
}
