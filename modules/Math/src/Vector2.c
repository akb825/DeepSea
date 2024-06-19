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

#include <DeepSea/Math/Vector2.h>

void dsVector2f_add(dsVector2f* result, const dsVector2f* a, const dsVector2f* b);
void dsVector2d_add(dsVector2d* result, const dsVector2d* a, const dsVector2d* b);
void dsVector2i_add(dsVector2i* result, const dsVector2i* a, const dsVector2i* b);
void dsVector2l_add(dsVector2l* result, const dsVector2l* a, const dsVector2l* b);

void dsVector2f_sub(dsVector2f* result, const dsVector2f* a, const dsVector2f* b);
void dsVector2d_sub(dsVector2d* result, const dsVector2d* a, const dsVector2d* b);
void dsVector2i_sub(dsVector2i* result, const dsVector2i* a, const dsVector2i* b);
void dsVector2l_sub(dsVector2l* result, const dsVector2l* a, const dsVector2l* b);

void dsVector2f_mul(dsVector2f* result, const dsVector2f* a, const dsVector2f* b);
void dsVector2d_mul(dsVector2d* result, const dsVector2d* a, const dsVector2d* b);
void dsVector2i_mul(dsVector2i* result, const dsVector2i* a, const dsVector2i* b);
void dsVector2l_mul(dsVector2l* result, const dsVector2l* a, const dsVector2l* b);

void dsVector2f_div(dsVector2f* result, const dsVector2f* a, const dsVector2f* b);
void dsVector2d_div(dsVector2d* result, const dsVector2d* a, const dsVector2d* b);
void dsVector2i_div(dsVector2i* result, const dsVector2i* a, const dsVector2i* b);
void dsVector2l_div(dsVector2l* result, const dsVector2l* a, const dsVector2l* b);

void dsVector2f_scale(dsVector2f* result, const dsVector2f* a, float s);
void dsVector2d_scale(dsVector2d* result, const dsVector2d* a, double s);
void dsVector2i_scale(dsVector2i* result, const dsVector2i* a, int s);
void dsVector2l_scale(dsVector2l* result, const dsVector2l* a, long long s);

void dsVector2f_neg(dsVector2f* result, const dsVector2f* a);
void dsVector2d_neg(dsVector2d* result, const dsVector2d* a);
void dsVector2i_neg(dsVector2i* result, const dsVector2i* a);
void dsVector2l_neg(dsVector2l* result, const dsVector2l* a);

void dsVector2f_lerp(dsVector2f* result, const dsVector2f* a, const dsVector2f* b, float t);
void dsVector2d_lerp(dsVector2d* result, const dsVector2d* a, const dsVector2d* b, double t);
void dsVector2i_lerp(dsVector2i* result, const dsVector2i* a, const dsVector2i* b, float t);
void dsVector2l_lerp(dsVector2l* result, const dsVector2l* a, const dsVector2l* b, double t);

float dsVector2f_dot(const dsVector2f* a, const dsVector2f* b);
double dsVector2d_dot(const dsVector2d* a, const dsVector2d* b);
int dsVector2i_dot(const dsVector2i* a, const dsVector2i* b);
long long dsVector2l_dot(const dsVector2l* a, const dsVector2l* b);

float dsVector2f_len2(const dsVector2f* a);
double dsVector2d_len2(const dsVector2d* a);
int dsVector2i_len2(const dsVector2i* a);
long long dsVector2l_len2(const dsVector2l* a);

float dsVector2f_dist2(const dsVector2f* a, const dsVector2f* b);
double dsVector2d_dist2(const dsVector2d* a, const dsVector2d* b);
int dsVector2i_dist2(const dsVector2i* a, const dsVector2i* b);
long long dsVector2l_dist2(const dsVector2l* a, const dsVector2l* b);

bool dsVector2f_equal(const dsVector2f* a, const dsVector2f* b);
bool dsVector2d_equal(const dsVector2d* a, const dsVector2d* b);
bool dsVector2i_equal(const dsVector2i* a, const dsVector2i* b);
bool dsVector2l_equal(const dsVector2l* a, const dsVector2l* b);

float dsVector2f_len(const dsVector2f* a);
double dsVector2d_len(const dsVector2d* a);
double dsVector2i_len(const dsVector2i* a);
double dsVector2l_len(const dsVector2l* a);

float dsVector2f_dist(const dsVector2f* a, const dsVector2f* b);
double dsVector2d_dist(const dsVector2d* a, const dsVector2d* b);
double dsVector2i_dist(const dsVector2i* a, const dsVector2i* b);
double dsVector2i_dist(const dsVector2i* a, const dsVector2i* b);

void dsVector2f_normalize(dsVector2f* result, const dsVector2f* a);
void dsVector2d_normalize(dsVector2d* result, const dsVector2d* a);

bool dsVector2f_epsilonEqual(const dsVector2f* a, const dsVector2f* b, float epsilon);
bool dsVector2d_epsilonEqual(const dsVector2d* a, const dsVector2d* b, double epsilon);

bool dsVector2f_relativeEpsilonEqual(const dsVector2f* a, const dsVector2f* b, float absoluteEps,
	float relativeEps);
bool dsVector2d_relativeEpsilonEqual(const dsVector2d* a, const dsVector2d* b, double absoluteEps,
	double relativeEps);
