/*
 * Copyright 2018-2026 Aaron Barany
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
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Memory/Types.h>

#if DS_WINDOWS
#include <malloc.h>
#else
#include <alloca.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros for allocating an objects on the stack.
 */

/**
 * @brief Macro to allocate an object on the stack and return it as a pointer to that object type.
 * @remark errno will be set on failure.
 * @param type The type to allocate.
 * @return The allocated object.
 */
#define DS_ALLOCATE_STACK_OBJECT(type) ((type*)alloca(sizeof(type)))

/**
 * @brief Macro to allocate an array of objects on the stack and return it as a pointer to that
 *     object type.
 * @remark errno will be set on failure.
 * @param type The type to allocate.
 * @param count The number of objects to allocate.
 * @return The allocated array.
 */
#define DS_ALLOCATE_STACK_OBJECT_ARRAY(type, count) ((type*)alloca(sizeof(type)*(count)))

#ifdef __cplusplus
}
#endif
