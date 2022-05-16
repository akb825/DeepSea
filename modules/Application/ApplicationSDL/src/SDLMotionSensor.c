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

#include "SDLMotionSensor.h"
#include <DeepSea/Application/Application.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

#if SDL_VERSION_ATLEAST(2, 0, 9)

static dsMotionSensor* createMotionSensor(dsApplication* application, uint32_t index,
	SDL_SensorType sdlType)
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
	baseSensor->name = SDL_SensorGetDeviceName(index);
	baseSensor->type = type;

	sensor->sensor = SDL_SensorOpen(index);
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
	SDL_SensorClose(sdlSensor->sensor);
	DS_VERIFY(dsAllocator_free(sensor->allocator, sensor));
}

#endif

bool dsSDLMotionSensor_setup(dsApplication* application)
{
#if SDL_VERSION_ATLEAST(2, 0, 9)
	DS_ASSERT(!application->motionSensors);
	DS_ASSERT(application->motionSensorCount == 0);
	uint32_t totalSensorCount = SDL_NumSensors();
	uint32_t sensorCount = 0;
	for (uint32_t i = 0; i < totalSensorCount; ++i)
	{
		switch (SDL_SensorGetDeviceType(i))
		{
			case SDL_SENSOR_ACCEL:
			case SDL_SENSOR_GYRO:
				++sensorCount;
			default:
				break;
		}
	}
	if (sensorCount == 0)
		return true;

	dsMotionSensor** sensors =  DS_ALLOCATE_OBJECT_ARRAY(application->allocator, dsMotionSensor*,
		sensorCount);
	if (!sensors)
		return false;

	for (uint32_t i = 0, sensorIndex = 0; sensorIndex < totalSensorCount; ++sensorIndex)
	{
		SDL_SensorType sdlType = SDL_SensorGetDeviceType(sensorIndex);
		if (sdlType != SDL_SENSOR_ACCEL && sdlType != SDL_SENSOR_GYRO)
			continue;

		dsMotionSensor* sensor = createMotionSensor(application, sensorIndex, sdlType);
		if (!sensor)
		{
			dsSDLMotionSensor_freeAll(sensors, i);
			DS_VERIFY(dsAllocator_free(application->allocator, sensors));
			return false;
		}

		sensors[i] = sensor;
		++i;
	}

	application->motionSensors = sensors;
	application->motionSensorCount = sensorCount;
	application->motionSensorCapacity = sensorCount;
	return true;
#else
	DS_UNUSED(application);
	return true;
#endif
}

void dsSDLMotionSensor_freeAll(dsMotionSensor** sensors, uint32_t sensorCount)
{
#if SDL_VERSION_ATLEAST(2, 0, 9)
	if (!sensors)
		return;

	for (uint32_t i = 0; i < sensorCount; ++i)
		freeMotionSensor(sensors[i]);
#else
	DS_UNUSED(sensors);
	DS_UNUSED(sensorCount);
#endif
}

dsMotionSensor* dsSDLMotionSensor_add(dsApplication* application, uint32_t index)
{
#if SDL_VERSION_ATLEAST(2, 0, 9)
	dsMotionSensor* sensor = createMotionSensor(application, index, SDL_SensorGetDeviceType(index));
	if (!sensor)
		return NULL;

	if (!dsApplication_addMotionSensor(application, sensor))
	{
		freeMotionSensor(sensor);
		return NULL;
	}

	return sensor;
#else
	DS_UNUSED(application);
	DS_UNUSED(index);
	errno = EPERM;
	return NULL;
#endif
}

bool dsSDLMotionSensor_remove(dsApplication* application, SDL_SensorID id)
{
#if SDL_VERSION_ATLEAST(2, 0, 9)
	dsMotionSensor* sensor = dsSDLMotionSensor_find(application, id);

	if (!dsApplication_removeMotionSensor(application, sensor))
		return false;

	freeMotionSensor(sensor);
	return true;
#else
	DS_UNUSED(application);
	DS_UNUSED(id);
	errno = EPERM;
	return false;
#endif
}

dsMotionSensor* dsSDLMotionSensor_find(dsApplication* application, SDL_SensorID id)
{
#if SDL_VERSION_ATLEAST(2, 0, 9)
	for (uint32_t i = 0; i < application->motionSensorCount; ++i)
	{
		if (SDL_SensorGetInstanceID(
				((dsSDLMotionSensor*)application->motionSensors[i])->sensor) == id)
		{
			return application->motionSensors[i];
		}
	}

	return NULL;
#else
	DS_UNUSED(application);
	DS_UNUSED(id);
	return false;
#endif
}

bool dsSDLMotionSensor_getData(dsVector3f* outData, const dsApplication* application,
	const dsMotionSensor* sensor)
{
#if SDL_VERSION_ATLEAST(2, 0, 9)
	const dsSDLMotionSensor* sdlSensor = (const dsSDLMotionSensor*)sensor;
	if (SDL_SensorGetData(sdlSensor->sensor, (float*)outData, 3) != 0)
	{
		errno = EPERM;
		return false;
	}

	return true;
#else
	DS_UNUSED(outData);
	DS_UNUSED(application);
	DS_UNUSED(sensor);
	errno = EPERM;
	return false;
#endif
}
