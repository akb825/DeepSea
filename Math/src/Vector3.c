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

#include <DeepSea/Math/Vector3.h>
#include <DeepSea/Math/Export.h>

DS_MATH_EXPORT float dsVector3f_len(const dsVector3f* a);
DS_MATH_EXPORT double dsVector3d_len(const dsVector3d* a);

DS_MATH_EXPORT float dsVector3f_dist(const dsVector3f* a, const dsVector3f* b);
DS_MATH_EXPORT double dsVector3d_dist(const dsVector3d* a, const dsVector3d* b);

DS_MATH_EXPORT void dsVector3f_normalize(dsVector3f* result, const dsVector3f* a);
DS_MATH_EXPORT void dsVector3d_normalize(dsVector3d* result, const dsVector3d* a);
