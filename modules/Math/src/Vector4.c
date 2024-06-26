/*
 * Copyright 2016-2024 Aaron Barany
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

void dsVector4f_add(dsVector4f* result, const dsVector4f* a, const dsVector4f* b);
void dsVector4d_add(dsVector4d* result, const dsVector4d* a, const dsVector4d* b);
void dsVector4i_add(dsVector4i* result, const dsVector4i* a, const dsVector4i* b);
void dsVector4l_add(dsVector4l* result, const dsVector4l* a, const dsVector4l* b);

void dsVector4f_sub(dsVector4f* result, const dsVector4f* a, const dsVector4f* b);
void dsVector4d_sub(dsVector4d* result, const dsVector4d* a, const dsVector4d* b);
void dsVector4i_sub(dsVector4i* result, const dsVector4i* a, const dsVector4i* b);
void dsVector4l_sub(dsVector4l* result, const dsVector4l* a, const dsVector4l* b);

void dsVector4f_mul(dsVector4f* result, const dsVector4f* a, const dsVector4f* b);
void dsVector4d_mul(dsVector4d* result, const dsVector4d* a, const dsVector4d* b);
void dsVector4i_mul(dsVector4i* result, const dsVector4i* a, const dsVector4i* b);
void dsVector4l_mul(dsVector4l* result, const dsVector4l* a, const dsVector4l* b);

void dsVector4f_div(dsVector4f* result, const dsVector4f* a, const dsVector4f* b);
void dsVector4d_div(dsVector4d* result, const dsVector4d* a, const dsVector4d* b);
void dsVector4i_div(dsVector4i* result, const dsVector4i* a, const dsVector4i* b);
void dsVector4l_div(dsVector4l* result, const dsVector4l* a, const dsVector4l* b);

void dsVector4f_scale(dsVector4f* result, const dsVector4f* a, float s);
void dsVector4d_scale(dsVector4d* result, const dsVector4d* a, double s);
void dsVector4i_scale(dsVector4i* result, const dsVector4i* a, int s);
void dsVector4l_scale(dsVector4l* result, const dsVector4l* a, long long s);

void dsVector4f_neg(dsVector4f* result, const dsVector4f* a);
void dsVector4d_neg(dsVector4d* result, const dsVector4d* a);
void dsVector4i_neg(dsVector4i* result, const dsVector4i* a);
void dsVector4l_neg(dsVector4l* result, const dsVector4l* a);

void dsVector4f_lerp(dsVector4f* result, const dsVector4f* a, const dsVector4f* b, float t);
void dsVector4d_lerp(dsVector4d* result, const dsVector4d* a, const dsVector4d* b, double t);
void dsVector4i_lerp(dsVector4i* result, const dsVector4i* a, const dsVector4i* b, float t);
void dsVector4l_lerp(dsVector4l* result, const dsVector4l* a, const dsVector4l* b, double t);

float dsVector4f_dot(const dsVector4f* a, const dsVector4f* b);
double dsVector4d_dot(const dsVector4d* a, const dsVector4d* b);
int dsVector4i_dot(const dsVector4i* a, const dsVector4i* b);
long long dsVector4l_dot(const dsVector4l* a, const dsVector4l* b);

float dsVector4f_len2(const dsVector4f* a);
double dsVector4d_len2(const dsVector4d* a);
int dsVector4i_len2(const dsVector4i* a);
long long dsVector4l_len2(const dsVector4l* a);

float dsVector4f_dist2(const dsVector4f* a, const dsVector4f* b);
double dsVector4d_dist2(const dsVector4d* a, const dsVector4d* b);
int dsVector4i_dist2(const dsVector4i* a, const dsVector4i* b);
long long dsVector4l_dist2(const dsVector4l* a, const dsVector4l* b);

bool dsVector4f_equal(const dsVector4f* a, const dsVector4f* b);
bool dsVector4d_equal(const dsVector4d* a, const dsVector4d* b);
bool dsVector4i_equal(const dsVector4i* a, const dsVector4i* b);
bool dsVector4l_equal(const dsVector4l* a, const dsVector4l* b);

float dsVector4f_len(const dsVector4f* a);
double dsVector4d_len(const dsVector4d* a);
double dsVector4i_len(const dsVector4i* a);
double dsVector4l_len(const dsVector4l* a);

float dsVector4f_dist(const dsVector4f* a, const dsVector4f* b);
double dsVector4d_dist(const dsVector4d* a, const dsVector4d* b);
double dsVector4i_dist(const dsVector4i* a, const dsVector4i* b);
double dsVector4l_dist(const dsVector4l* a, const dsVector4l* b);

void dsVector4f_normalize(dsVector4f* result, const dsVector4f* a);
void dsVector4d_normalize(dsVector4d* result, const dsVector4d* a);

bool dsVector4f_epsilonEqual(const dsVector4f* a, const dsVector4f* b, float epsilon);
bool dsVector4d_epsilonEqual(const dsVector4d* a, const dsVector4d* b, double epsilon);

bool dsVector4f_relativeEpsilonEqual(const dsVector4f* a, const dsVector4f* b, float absoluteEps,
	float relativeEps);
bool dsVector4d_relativeEpsilonEqual(const dsVector4d* a, const dsVector4d* b, double absoluteEps,
	double relativeEps);
