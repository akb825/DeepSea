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

#pragma once

#include <DeepSea/Core/Types.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Render/Types.h>
#include <DeepSea/RenderBootstrap/Types.h>
#include <gtest/gtest.h>

class FixtureBase : public testing::TestWithParam<dsRendererType>
{
public:
	static testing::internal::ParamGenerator<dsRendererType> getRendererTypes();

	void SetUp() override;
	void TearDown() override;

	const char* getShaderPath(const char* fileName) const;

	dsSystemAllocator allocator;
	dsRenderer* renderer;
	dsResourceManager* resourceManager;

private:
	char m_shaderDir[DS_PATH_MAX];
	mutable char m_buffer[DS_PATH_MAX];
};
