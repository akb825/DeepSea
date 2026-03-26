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

#include <DeepSea/Math/Trig.h>

float dsSinf(float angle);
double dsSind(double angle);
float dsCosf(float angle);
double dsCosd(double angle);
void dsSinCosf(float* outSin, float* outCos, float angle);
void dsSinCosd(double* outSin, double* outCos, double angle);
float dsTanf(float angle);
double dsTand(double angle);
float dsASinf(float x);
double dsASind(double x);
float dsACosf(float x);
double dsACosd(double x);
float dsATanf(float x);
double dsATand(double x);
float dsATan2f(float y, float x);
double dsATan2d(double y, double x);

#if DS_HAS_SIMD

dsSIMD4f dsSinSIMD4f(dsSIMD4f angles);
dsSIMD4f dsCosSIMD4f(dsSIMD4f angles);
void dsSinCosSIMD4f(dsSIMD4f* outSin, dsSIMD4f* outCos, dsSIMD4f angles);
dsSIMD4f dsTanSIMD4f(dsSIMD4f angles);
dsSIMD4f dsASinSIMD4f(dsSIMD4f x);
dsSIMD4f dsACosSIMD4f(dsSIMD4f x);
dsSIMD4f dsATanSIMD4f(dsSIMD4f x);
dsSIMD4f dsATan2SIMD4f(dsSIMD4f y, dsSIMD4f x);

#if !DS_DETERMINISTIC_MATH
dsSIMD4f dsSinFMA4f(dsSIMD4f angles);
dsSIMD4f dsCosFMA4f(dsSIMD4f angles);
void dsSinCosFMA4f(dsSIMD4f* outSin, dsSIMD4f* outCos, dsSIMD4f angles);
dsSIMD4f dsTanFMA4f(dsSIMD4f angles);
dsSIMD4f dsASinFMA4f(dsSIMD4f x);
dsSIMD4f dsACosFMA4f(dsSIMD4f x);
dsSIMD4f dsATanFMA4f(dsSIMD4f x);
dsSIMD4f dsATan2FMA4f(dsSIMD4f y, dsSIMD4f x);
#endif

dsSIMD2d dsSinSIMD2d(dsSIMD2d angles);
dsSIMD2d dsCosSIMD2d(dsSIMD2d angles);
void dsSinCosSIMD2d(dsSIMD2d* outSin, dsSIMD2d* outCos, dsSIMD2d angles);
dsSIMD2d dsTanSIMD2d(dsSIMD2d angles);
dsSIMD2d dsASinSIMD2d(dsSIMD2d x);
dsSIMD2d dsACosSIMD2d(dsSIMD2d x);
dsSIMD2d dsATanSIMD2d(dsSIMD2d x);
dsSIMD2d dsATan2SIMD2d(dsSIMD2d y, dsSIMD2d x);

#if !DS_DETERMINISTIC_MATH
dsSIMD2d dsSinFMA2d(dsSIMD2d angles);
dsSIMD2d dsCosFMA2d(dsSIMD2d angles);
void dsSinCosFMA2d(dsSIMD2d* outSin, dsSIMD2d* outCos, dsSIMD2d angles);
dsSIMD2d dsTanFMA2d(dsSIMD2d angles);
dsSIMD2d dsASinFMA2d(dsSIMD2d x);
dsSIMD2d dsACosFMA2d(dsSIMD2d x);
dsSIMD2d dsATanFMA2d(dsSIMD2d x);
dsSIMD2d dsATan2FMA2d(dsSIMD2d y, dsSIMD2d x);
#endif

dsSIMD4d dsSinSIMD4d(dsSIMD4d angles);
dsSIMD4d dsCosSIMD4d(dsSIMD4d angles);
void dsSinCosSIMD4d(dsSIMD4d* outSin, dsSIMD4d* outCos, dsSIMD4d angles);
dsSIMD4d dsTanSIMD4d(dsSIMD4d angles);
dsSIMD4d dsASinSIMD4d(dsSIMD4d x);
dsSIMD4d dsACosSIMD4d(dsSIMD4d x);
dsSIMD4d dsATanSIMD4d(dsSIMD4d x);
dsSIMD4d dsATan2SIMD4d(dsSIMD4d y, dsSIMD4d x);

#endif
