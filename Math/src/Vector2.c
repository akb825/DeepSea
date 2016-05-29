/*
 * Copyright 2016 Aaron Barany
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

#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Math/Export.h>

DS_MATH_EXPORT float dsVector2f_len(const dsVector2f* a);
DS_MATH_EXPORT double dsVector2d_len(const dsVector2d* a);

DS_MATH_EXPORT float dsVector2f_dist(const dsVector2f* a, const dsVector2f* b);
DS_MATH_EXPORT double dsVector2d_dist(const dsVector2d* a, const dsVector2d* b);

DS_MATH_EXPORT void dsVector2f_normalize(dsVector2f* result, const dsVector2f* a);
DS_MATH_EXPORT void dsVector2d_normalize(dsVector2d* result, const dsVector2d* a);
