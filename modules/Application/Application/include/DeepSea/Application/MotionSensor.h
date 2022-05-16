/*
 * Copyright 2022 Aaron Barany
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
#include <DeepSea/Application/Export.h>
#include <DeepSea/Application/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for working with motion sensors.
 * @see dsMotionSensor
 */

/**
 * @brief Gets the data from a motion sensor.
 * @remark errno will be set on failure.
 * @param[out] outData The data for the motion sensor.
 * @param sensor The motion sensor to get the data from.
 * @return False if the data couldn't be queried.
 */
DS_APPLICATION_EXPORT bool dsMotionSensor_getData(dsVector3f* outData,
	const dsMotionSensor* sensor);

#ifdef __cplusplus
}
#endif
