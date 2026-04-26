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

#include <DeepSea/Math/Exponent.h>

float dsSplitPow2f(int* outPow2, float x);
double dsSplitPow2d(int* outPow2, double x);
float dsMulPow2f(float x, int pow2);
double dsMulPow2d(double x, int pow2);
float dsExpf(float x);
double dsExpd(double x);
float dsExp2f(float x);
double dsExp2d(double x);

#if DS_HAS_SIMD

dsSIMD4f dsSplitPow2SIMD4f(dsSIMD4fb* outPow2, dsSIMD4f x);
dsSIMD4f dsMulPow2SIMD4f(dsSIMD4f x, dsSIMD4fb pow2);
dsSIMD4f dsExpSIMD4f(dsSIMD4f x);
dsSIMD4f dsExp2SIMD4f(dsSIMD4f x);

#if !DS_DETERMINISTIC_MATH
dsSIMD4f dsExpFMA4f(dsSIMD4f x);
dsSIMD4f dsExp2FMA4f(dsSIMD4f x);
#endif // DS_DETERMINISTIC_MATH

dsSIMD2d dsSplitPow2SIMD2d(dsSIMD2db* outPow2, dsSIMD2d x);
dsSIMD2d dsMulPow2SIMD2d(dsSIMD2d x, dsSIMD2db pow2);
dsSIMD2d dsExpSIMD2d(dsSIMD2d x);
dsSIMD2d dsExp2SIMD2d(dsSIMD2d x);

#if !DS_DETERMINISTIC_MATH
dsSIMD2d dsExpFMA2d(dsSIMD2d x);
dsSIMD2d dsExp2FMA2d(dsSIMD2d x);
#endif // DS_DETERMINISTIC_MATH

dsSIMD4d dsSplitPow2SIMD4d(dsSIMD4db* outPow2, dsSIMD4d x);
dsSIMD4d dsMulPow2SIMD4d(dsSIMD4d x, dsSIMD4db pow2);
dsSIMD4d dsExpSIMD4d(dsSIMD4d x);
dsSIMD4d dsExp2SIMD4d(dsSIMD4d x);

#endif // DS_HAS_SIMD
