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

#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <gtest/gtest.h>

#if DS_WINDOWS
extern "C"
{
	// Mimick the "real world" case of preferring the descrete GPU. This also helps where Intel
	// drivers are a complete bugfest and fail some of the bootstrap tests.
	__declspec(dllexport) uint32_t NvOptimusEnablement = 1;
	__declspec(dllexport) uint32_t AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);

#if !DS_ANDROID
	char testerDir[DS_PATH_MAX];
	dsPath_getDirectoryName(testerDir, sizeof(testerDir), argv[0]);
	dsResourceStream_setContext(NULL, NULL, testerDir, NULL, NULL);
#endif

	return RUN_ALL_TESTS();
}
