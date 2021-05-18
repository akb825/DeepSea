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

void dsVector3f_add(dsVector3f* result, const dsVector3f* a, const dsVector3f* b);
void dsVector3d_add(dsVector3d* result, const dsVector3d* a, const dsVector3d* b);
void dsVector3i_add(dsVector3i* result, const dsVector3i* a, const dsVector3i* b);

void dsVector3f_sub(dsVector3f* result, const dsVector3f* a, const dsVector3f* b);
void dsVector3d_sub(dsVector3d* result, const dsVector3d* a, const dsVector3d* b);
void dsVector3i_sub(dsVector3i* result, const dsVector3i* a, const dsVector3i* b);

void dsVector3f_mul(dsVector3f* result, const dsVector3f* a, const dsVector3f* b);
void dsVector3d_mul(dsVector3d* result, const dsVector3d* a, const dsVector3d* b);
void dsVector3i_mul(dsVector3i* result, const dsVector3i* a, const dsVector3i* b);

void dsVector3f_div(dsVector3f* result, const dsVector3f* a, const dsVector3f* b);
void dsVector3d_div(dsVector3d* result, const dsVector3d* a, const dsVector3d* b);
void dsVector3i_div(dsVector3i* result, const dsVector3i* a, const dsVector3i* b);

void dsVector3f_scale(dsVector3f* result, const dsVector3f* a, float s);
void dsVector3d_scale(dsVector3d* result, const dsVector3d* a, double s);
void dsVector3i_scale(dsVector3i* result, const dsVector3i* a, int s);

void dsVector3f_neg(dsVector3f* result, const dsVector3f* a);
void dsVector3d_neg(dsVector3d* result, const dsVector3d* a);
void dsVector3i_neg(dsVector3i* result, const dsVector3i* a);

void dsVector3f_lerp(dsVector3f* result, const dsVector3f* a, const dsVector3f* b, float t);
void dsVector3d_lerp(dsVector3d* result, const dsVector3d* a, const dsVector3d* b, double t);
void dsVector3i_lerp(dsVector3i* result, const dsVector3i* a, const dsVector3i* b, float t);

float dsVector3f_dot(const dsVector3f* a, const dsVector3f* b);
double dsVector3d_dot(const dsVector3d* a, const dsVector3d* b);
int dsVector3i_dot(const dsVector3i* a, const dsVector3i* b);

void dsVector3f_cross(dsVector3f* result, const dsVector3f* a, const dsVector3f* b);
void dsVector3d_cross(dsVector3d* result, const dsVector3d* a, const dsVector3d* b);
void dsVector3i_cross(dsVector3i* result, const dsVector3i* a, const dsVector3i* b);

float dsVector3f_len2(const dsVector3f* a);
double dsVector3d_len2(const dsVector3d* a);
int dsVector3i_len2(const dsVector3i* a);

float dsVector3f_dist2(const dsVector3f* a, const dsVector3f* b);
double dsVector3d_dist2(const dsVector3d* a, const dsVector3d* b);
int dsVector3i_dist2(const dsVector3i* a, const dsVector3i* b);

bool dsVector3f_equal(const dsVector3f* a, const dsVector3f* b);
bool dsVector3d_equal(const dsVector3d* a, const dsVector3d* b);
bool dsVector3i_equal(const dsVector3i* a, const dsVector3i* b);

float dsVector3f_len(const dsVector3f* a);
double dsVector3d_len(const dsVector3d* a);
double dsVector3i_len(const dsVector3i* a);

float dsVector3f_dist(const dsVector3f* a, const dsVector3f* b);
double dsVector3d_dist(const dsVector3d* a, const dsVector3d* b);
double dsVector3i_dist(const dsVector3i* a, const dsVector3i* b);

void dsVector3f_normalize(dsVector3f* result, const dsVector3f* a);
void dsVector3d_normalize(dsVector3d* result, const dsVector3d* a);

bool dsVector3f_epsilonEqual(const dsVector3f* a, const dsVector3f* b, float epsilon);
bool dsVector3d_epsilonEqual(const dsVector3d* a, const dsVector3d* b, double epsilon);

bool dsVector3f_relativeEpsilonEqual(const dsVector3f* a, const dsVector3f* b, float epsilon);
bool dsVector3d_relativeEpsilonEqual(const dsVector3d* a, const dsVector3d* b, double epsilon);
