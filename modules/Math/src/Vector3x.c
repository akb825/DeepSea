/*
 * Copyright 2026 Aaron Barany
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

#include <DeepSea/Math/Vector3x.h>

void dsVector3xf_add(dsVector3xf* result, const dsVector3xf* a, const dsVector3xf* b);
void dsVector3xd_add(dsVector3xd* result, const dsVector3xd* a, const dsVector3xd* b);

void dsVector3xf_sub(dsVector3xf* result, const dsVector3xf* a, const dsVector3xf* b);
void dsVector3xd_sub(dsVector3xd* result, const dsVector3xd* a, const dsVector3xd* b);

void dsVector3xf_mul(dsVector3xf* result, const dsVector3xf* a, const dsVector3xf* b);
void dsVector3xd_mul(dsVector3xd* result, const dsVector3xd* a, const dsVector3xd* b);

void dsVector3xf_div(dsVector3xf* result, const dsVector3xf* a, const dsVector3xf* b);
void dsVector3xd_div(dsVector3xd* result, const dsVector3xd* a, const dsVector3xd* b);

void dsVector3xf_scale(dsVector3xf* result, const dsVector3xf* a, float s);
void dsVector3xd_scale(dsVector3xd* result, const dsVector3xd* a, double s);

void dsVector3xf_neg(dsVector3xf* result, const dsVector3xf* a);
void dsVector3xd_neg(dsVector3xd* result, const dsVector3xd* a);

void dsVector3xf_lerp(dsVector3xf* result, const dsVector3xf* a, const dsVector3xf* b, float t);
void dsVector3xd_lerp(dsVector3xd* result, const dsVector3xd* a, const dsVector3xd* b, double t);

float dsVector3xf_dot(const dsVector3xf* a, const dsVector3xf* b);
double dsVector3xd_dot(const dsVector3xd* a, const dsVector3xd* b);

void dsVector3xf_cross(dsVector3xf* result, const dsVector3xf* a, const dsVector3xf* b);
void dsVector3xd_cross(dsVector3xd* result, const dsVector3xd* a, const dsVector3xd* b);

float dsVector3xf_len2(const dsVector3xf* a);
double dsVector3xd_len2(const dsVector3xd* a);

float dsVector3xf_dist2(const dsVector3xf* a, const dsVector3xf* b);
double dsVector3xd_dist2(const dsVector3xd* a, const dsVector3xd* b);

bool dsVector3xf_equal(const dsVector3xf* a, const dsVector3xf* b);
bool dsVector3xd_equal(const dsVector3xd* a, const dsVector3xd* b);

float dsVector3xf_len(const dsVector3xf* a);
double dsVector3xd_len(const dsVector3xd* a);

float dsVector3xf_dist(const dsVector3xf* a, const dsVector3xf* b);
double dsVector3xd_dist(const dsVector3xd* a, const dsVector3xd* b);

void dsVector3xf_normalize(dsVector3xf* result, const dsVector3xf* a);
void dsVector3xd_normalize(dsVector3xd* result, const dsVector3xd* a);

bool dsVector3xf_epsilonEqual(const dsVector3xf* a, const dsVector3xf* b, float epsilon);
bool dsVector3xd_epsilonEqual(const dsVector3xd* a, const dsVector3xd* b, double epsilon);

bool dsVector3xf_relativeEpsilonEqual(
	const dsVector3xf* a, const dsVector3xf* b, float absoluteEps, float relativeEps);
bool dsVector3xd_relativeEpsilonEqual(
	const dsVector3xd* a, const dsVector3xd* b, double absoluteEps, double relativeEps);
