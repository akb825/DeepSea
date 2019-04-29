/*
 * Copyright 2019 Aaron Barany
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

#include "MTLShared.h"

MTLCompareFunction dsGetMTLCompareFunction(mslCompareOp compare)
{
	switch (compare)
	{
		case mslCompareOp_Less:
			return MTLCompareFunctionLess;
		case mslCompareOp_Equal:
			return MTLCompareFunctionEqual;
		case mslCompareOp_LessOrEqual:
			return MTLCompareFunctionLessEqual;
		case mslCompareOp_Greater:
			return MTLCompareFunctionGreater;
		case mslCompareOp_NotEqual:
			return MTLCompareFunctionNotEqual;
		case mslCompareOp_GreaterOrEqual:
			return MTLCompareFunctionGreaterEqual;
		case mslCompareOp_Always:
			return MTLCompareFunctionAlways;
		case mslCompareOp_Never:
		default:
			return MTLCompareFunctionNever;
	}
}

MTLStencilOperation dsGetMTLStencilOp(mslStencilOp op)
{
	switch (op)
	{
		case mslStencilOp_Zero:
			return MTLStencilOperationZero;
		case mslStencilOp_Replace:
			return MTLStencilOperationReplace;
		case mslStencilOp_IncrementAndClamp:
			return MTLStencilOperationIncrementClamp;
		case mslStencilOp_DecrementAndClamp:
			return MTLStencilOperationDecrementClamp;
		case mslStencilOp_Invert:
			return MTLStencilOperationInvert;
		case mslStencilOp_IncrementAndWrap:
			return MTLStencilOperationIncrementWrap;
		case mslStencilOp_DecrementAndWrap:
			return MTLStencilOperationDecrementWrap;
		case mslStencilOp_Keep:
		default:
			return MTLStencilOperationKeep;
	}
}
