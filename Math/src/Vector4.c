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

#include <DeepSea/Math/Vector4.h>
#include <DeepSea/Math/Export.h>

DS_MATH_EXPORT float dsVector4f_len(const dsVector4f* a);
DS_MATH_EXPORT double dsVector4d_len(const dsVector4d* a);

DS_MATH_EXPORT float dsVector4f_dist(const dsVector4f* a, const dsVector4f* b);
DS_MATH_EXPORT double dsVector4d_dist(const dsVector4d* a, const dsVector4d* b);

DS_MATH_EXPORT void dsVector4f_normalize(dsVector4f* result, const dsVector4f* a);
DS_MATH_EXPORT void dsVector4d_normalize(dsVector4d* result, const dsVector4d* a);
