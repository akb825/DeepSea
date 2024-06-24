/*
 * Copyright 2024 Aaron Barany
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

#include "RigidBodyGroupLoad.h"

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/RigidBodyGroup.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/RigidBodyGroup_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

dsRigidBodyGroup* dsRigidBodyGroup_loadImpl(dsPhysicsEngine* engine, dsAllocator* allocator,
	const void* data, size_t size, const char* name)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!DeepSeaPhysics::VerifyRigidBodyGroupBuffer(verifier))
	{
		errno = EFORMAT;
		if (name)
		{
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
				"Invalid rigid body group flatbuffer format for '%s'.", name);
		}
		else
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Invalid rigid body flatbuffer format.");
		return nullptr;
	}

	auto fbGroup = DeepSeaPhysics::GetRigidBodyGroup(data);
	auto motionType = static_cast<dsPhysicsMotionType>(fbGroup->motionType());
	return dsRigidBodyGroup_create(engine, allocator, motionType);
}
