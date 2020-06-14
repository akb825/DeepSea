/*
 * Copyright 2020 Aaron Barany
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

#include <DeepSea/Core/Config.h>
#include <DeepSea/Text/Export.h>
#include <DeepSea/Text/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for creating text substitution data.
 * @see dsTextSubstitutionData
 */

/**
 * @brief Creates data used during text substitution.
 * @param allocator The allocator to create the substitution data. This must support freeing
 *     memory.
 * @return The substitution data or NULL if an error occurred.
 */
DS_TEXT_EXPORT dsTextSubstitutionData* dsTextSubstitutionData_create(dsAllocator* allocator);

/**
 * @brief Destroys text substitution data.
 * @param data The data to destroy.
 */
DS_TEXT_EXPORT void dsTextSubstitutionData_destroy(dsTextSubstitutionData* data);

#ifdef __cplusplus
}
#endif
