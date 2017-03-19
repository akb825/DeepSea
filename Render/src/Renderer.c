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

#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Error.h>
#include <string.h>

bool dsRenderer_initialize(dsRenderer* renderer)
{
	if (!renderer)
	{
		errno = EINVAL;
		return false;
	}

	memset(renderer, 0, sizeof(dsRenderer));
	renderer->mainThread = dsThread_thisThreadId();
	return true;
}

void dsRenderer_shutdown(dsRenderer* renderer)
{
	DS_UNUSED(renderer);
}
