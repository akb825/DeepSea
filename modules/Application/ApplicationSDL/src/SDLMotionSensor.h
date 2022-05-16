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
#include <DeepSea/Application/Types.h>

#include <math.h>
#include <SDL.h>

#if !SDL_VERSION_ATLEAST(2, 0, 9)
typedef struct SDL_Sensor SDL_Sensor;
typedef Sint32 SDL_SensorID;
#endif

typedef struct dsSDLMotionSensor
{
	dsMotionSensor motionSensor;
	SDL_Sensor* sensor;
} dsSDLMotionSensor;

bool dsSDLMotionSensor_setup(dsApplication* application);
void dsSDLMotionSensor_freeAll(dsMotionSensor** sensors, uint32_t sensorCount);

dsMotionSensor* dsSDLMotionSensor_add(dsApplication* application, uint32_t index);
bool dsSDLMotionSensor_remove(dsApplication* application, SDL_SensorID id);
dsMotionSensor* dsSDLMotionSensor_find(dsApplication* application, SDL_SensorID id);

bool dsSDLMotionSensor_getData(dsVector3f* outData, const dsApplication* application,
	const dsMotionSensor* sensor);

