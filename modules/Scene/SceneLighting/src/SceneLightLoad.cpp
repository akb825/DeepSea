/*
 * Copyright 2022-2026 Aaron Barany
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

#include "SceneLightLoad.h"

#include <DeepSea/Core/Assert.h>

#include <DeepSea/Math/Trig.h>
#include <DeepSea/Math/Vector3x.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/SceneLighting/SceneLight.h>

namespace DeepSeaSceneLighting
{

bool extractLightData(dsSceneLight& light, LightUnion type, const void* obj)
{
	if (!obj)
		return false;

	switch (type)
	{
		case LightUnion::DirectionalLight:
		{
			auto& directionalLight = *reinterpret_cast<const DirectionalLight*>(obj);
			dsVector3xf direction = DeepSeaScene::convert3x(*directionalLight.direction());
			dsVector3xf_normalize(&direction, &direction);
			dsColor3f color = DeepSeaScene::convert(*directionalLight.color());
			DS_VERIFY(dsSceneLight_makeDirectional(
				&light, &direction, &color, directionalLight.intensity()));
			return true;
		}
		case LightUnion::PointLight:
		{
			auto& pointLight = *reinterpret_cast<const PointLight*>(obj);
			dsVector3xf position = DeepSeaScene::convert3x(*pointLight.position());
			dsColor3f color = DeepSeaScene::convert(*pointLight.color());
			return dsSceneLight_makePoint(&light, &position, &color, pointLight.intensity(),
				pointLight.linearFalloff(),pointLight.quadraticFalloff());
		}
		case LightUnion::SpotLight:
		{
			auto& spotLight = *reinterpret_cast<const SpotLight*>(obj);
			dsVector3xf position = DeepSeaScene::convert3x(*spotLight.position());
			dsVector3xf direction = DeepSeaScene::convert3x(*spotLight.direction());
			dsVector3xf_normalize(&direction, &direction);
			float cosInnerSpotAngle = dsCosf(spotLight.innerSpotAngle());
			float cosOuterSpotAngle = dsCosf(spotLight.outerSpotAngle());
			dsColor3f color = DeepSeaScene::convert(*spotLight.color());
			return dsSceneLight_makeSpot(&light, &position, &direction, &color,
				spotLight.intensity(), spotLight.linearFalloff(), spotLight.quadraticFalloff(),
				cosInnerSpotAngle, cosOuterSpotAngle);
		}
		default:
			return false;
	}
}

} // DeepSeaSceneLighting
