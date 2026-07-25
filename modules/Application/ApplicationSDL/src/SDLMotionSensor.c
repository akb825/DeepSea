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

#include "SDLMotionSensor.h"
#include <DeepSea/Application/Application.h>
#include <DeepSea/ApplicationSDL/Types.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

inline static bool isSensorSupported(SDL_SensorType sdlType)
{
	switch (sdlType)
	{
		case SDL_SENSOR_ACCEL:
		case SDL_SENSOR_GYRO:
		case SDL_SENSOR_ACCEL_L:
		case SDL_SENSOR_GYRO_L:
		case SDL_SENSOR_ACCEL_R:
		case SDL_SENSOR_GYRO_R:
			return true;
		default:
			return false;
	}
}

static dsMotionSensor* createMotionSensor(
	dsApplication* application, SDL_SensorID sensorID, SDL_SensorType sdlType)
{
	dsMotionSensorType type;
	switch (sdlType)
	{
		case SDL_SENSOR_ACCEL:
			type = dsMotionSensorType_Accelerometer;
			break;
		case SDL_SENSOR_GYRO:
			type = dsMotionSensorType_Gyroscope;
			break;
		case SDL_SENSOR_ACCEL_L:
			type = dsMotionSensorType_AccelerometerLeft;
			break;
		case SDL_SENSOR_GYRO_L:
			type = dsMotionSensorType_GyroscopeLeft;
			break;
		case SDL_SENSOR_ACCEL_R:
			type = dsMotionSensorType_AccelerometerRight;
			break;
		case SDL_SENSOR_GYRO_R:
			type = dsMotionSensorType_GyroscopeRight;
			break;
		default:
			errno = EINVAL;
			return NULL;
	}

	dsSDLMotionSensor* sensor = DS_ALLOCATE_OBJECT(application->allocator, dsSDLMotionSensor);
	if (!sensor)
		return NULL;

	dsMotionSensor* baseSensor = (dsMotionSensor*)sensor;
	baseSensor->application = application;
	baseSensor->allocator = application->allocator;
	baseSensor->name = SDL_GetSensorNameForID(sensorID);
	baseSensor->type = type;

	sensor->sensor = SDL_OpenSensor(sensorID);
	if (!sensor->sensor)
	{
		DS_VERIFY(dsAllocator_free(application->allocator, sensor));
		errno = ENOMEM;
		return NULL;
	}

	return baseSensor;
}

static void freeMotionSensor(dsMotionSensor* sensor)
{
	if (!sensor)
		return;

	dsSDLMotionSensor* sdlSensor = (dsSDLMotionSensor*)sensor;
	SDL_CloseSensor(sdlSensor->sensor);
	DS_VERIFY(dsAllocator_free(sensor->allocator, sensor));
}

bool dsSDLMotionSensor_setup(dsApplication* application)
{
	DS_ASSERT(!application->motionSensors);
	DS_ASSERT(application->motionSensorCount == 0);
	int totalSensorCount;
	SDL_SensorID* sdlSensors = SDL_GetSensors(&totalSensorCount);
	if (!sdlSensors)
	{
		DS_LOG_ERROR_F(
			DS_APPLICATION_SDL_LOG_TAG, "Couldn't get sensors: %s", SDL_GetError());
		errno = EPERM;
		return false;
	}

	uint32_t sensorCount = 0;
	for (int i = 0; i < totalSensorCount; ++i)
	{
		if (isSensorSupported(SDL_GetSensorTypeForID(sdlSensors[i])))
			++sensorCount;
	}
	if (sensorCount == 0)
	{
		SDL_free(sdlSensors);
		return true;
	}

	dsMotionSensor** sensors =  DS_ALLOCATE_OBJECT_ARRAY(
		application->allocator, dsMotionSensor*, sensorCount);
	if (!sensors)
	{
		SDL_free(sdlSensors);
		return false;
	}

	for (int i = 0, sdlIndex = 0; sdlIndex < totalSensorCount; ++sdlIndex)
	{
		SDL_SensorID sdlSensor = sdlSensors[sdlIndex];
		SDL_SensorType sdlType = SDL_GetSensorTypeForID(sdlSensor);
		if (!isSensorSupported(sdlType))
			continue;

		dsMotionSensor* sensor = createMotionSensor(application, sdlSensor, sdlType);
		if (!sensor)
		{
			dsSDLMotionSensor_freeAll(sensors, i);
			SDL_free(sdlSensors);
			DS_VERIFY(dsAllocator_free(application->allocator, sensors));
			return false;
		}

		sensors[i] = sensor;
		++i;
	}
	SDL_free(sdlSensors);

	application->motionSensors = sensors;
	application->motionSensorCount = sensorCount;
	application->motionSensorCapacity = sensorCount;
	return true;
}

void dsSDLMotionSensor_freeAll(dsMotionSensor** sensors, uint32_t sensorCount)
{
	if (!sensors)
		return;

	for (uint32_t i = 0; i < sensorCount; ++i)
		freeMotionSensor(sensors[i]);
}

dsMotionSensor* dsSDLMotionSensor_add(dsApplication* application, SDL_SensorID id)
{
	dsMotionSensor* sensor = createMotionSensor(application, id, SDL_GetSensorTypeForID(id));
	if (!sensor)
		return NULL;

	if (!dsApplication_addMotionSensor(application, sensor))
	{
		freeMotionSensor(sensor);
		return NULL;
	}

	return sensor;
}

bool dsSDLMotionSensor_remove(dsApplication* application, SDL_SensorID id)
{
	dsMotionSensor* sensor = dsSDLMotionSensor_find(application, id);

	if (!dsApplication_removeMotionSensor(application, sensor))
		return false;

	freeMotionSensor(sensor);
	return true;
}

dsMotionSensor* dsSDLMotionSensor_find(dsApplication* application, SDL_SensorID id)
{
	for (uint32_t i = 0; i < application->motionSensorCount; ++i)
	{
		if (SDL_GetSensorID(((dsSDLMotionSensor*)application->motionSensors[i])->sensor) == id)
			return application->motionSensors[i];
	}

	return NULL;
}

bool dsSDLMotionSensor_getData(
	dsVector3f* outData, const dsApplication* application, const dsMotionSensor* sensor)
{
	const dsSDLMotionSensor* sdlSensor = (const dsSDLMotionSensor*)sensor;
	if (!SDL_GetSensorData(sdlSensor->sensor, (float*)outData, 3))
	{
		errno = EPERM;
		return false;
	}

	return true;
}
