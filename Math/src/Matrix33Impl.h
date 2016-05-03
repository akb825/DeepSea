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

#pragma once

#define dsMatrix33_invertImpl(result, mat, invDet) \
	do \
	{ \
		(result).values[0][0] = ((mat).values[1][1]*(mat).values[2][2] - \
			(mat).values[1][2]*(mat).values[2][1])*invDet; \
		(result).values[0][1] = ((mat).values[1][0]*(mat).values[2][2] - \
			(mat).values[1][2]*(mat).values[2][0])*invDet; \
		(result).values[0][2] = ((mat).values[1][0]*(mat).values[2][1] - \
			(mat).values[1][1]*(mat).values[2][0])*invDet; \
		\
		(result).values[1][0] = ((mat).values[0][1]*(mat).values[2][2] - \
			(mat).values[0][2]*(mat).values[2][1])*invDet; \
		(result).values[1][1] = ((mat).values[0][0]*(mat).values[2][2] - \
			(mat).values[0][2]*(mat).values[2][0])*invDet; \
		(result).values[1][2] = ((mat).values[0][0]*(mat).values[2][1] - \
			(mat).values[0][1]*(mat).values[2][0])*invDet; \
		\
		(result).values[2][0] = ((mat).values[0][1]*(mat).values[1][2] - \
			(mat).values[0][2]*(mat).values[1][1])*invDet; \
		(result).values[2][1] = ((mat).values[0][0]*(mat).values[1][2] - \
			(mat).values[0][2]*(mat).values[1][0])*invDet; \
		(result).values[2][2] = ((mat).values[0][0]*(mat).values[1][1] - \
			(mat).values[0][1]*(mat).values[1][0])*invDet; \
	} while (0)

