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

#include <DeepSea/Application/Platform/WindowsHDRDisplayManager.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>

#include <string.h>

#if DS_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <dxgi.h>
#include <dxgi1_6.h>
#include <Windows.h>

typedef struct DisplayInfo
{
	HMONITOR monitor;
	float referenceNits;
	float maxNits;
	WCHAR deviceName[32];
} DisplayInfo;

struct dsWindowsHDRDisplayManager
{
	dsAllocator* allocator;

	DisplayInfo* displays;
	uint32_t displayCount;
	uint32_t maxDisplays;

	DISPLAYCONFIG_PATH_INFO* tempPaths;
	DISPLAYCONFIG_MODE_INFO* tempModes;
	uint32_t maxTempPaths;
	uint32_t maxTempModes;
};

// String forms of GUIDs can be found in the headers with the C++ interface declarations.
// 770aae78-f26f-4dba-a829-253c83d1b387
static const GUID dxgiFactory1ID =
	{0x770aae78, 0xf26f, 0x4dba, {0xa8, 0x29, 0x25, 0x3c, 0x83, 0xd1, 0xb3, 0x87}};
// 068346e8-aaec-4b84-add7-137f513f77a1
static const GUID dxgiOutput6ID =
	{0x068346e8, 0xaaec, 0x4b84, {0xad, 0xd7, 0x13, 0x7f, 0x51, 0x3f, 0x77, 0xa1}};

dsWindowsHDRDisplayManager* dsWindowsHDRDisplayManager_create(dsAllocator* allocator)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_APPLICATION_LOG_TAG,
			"Windows HDR display manager allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	dsWindowsHDRDisplayManager* displayManager = DS_ALLOCATE_OBJECT(
		allocator, dsWindowsHDRDisplayManager);
	if (!displayManager)
		return NULL;

	displayManager->allocator = allocator;
	displayManager->displays = NULL;
	displayManager->displayCount = 0;
	displayManager->maxDisplays = 0;
	displayManager->tempPaths = NULL;
	displayManager->tempModes = NULL;
	displayManager->maxTempPaths = 0;
	displayManager->maxTempModes = 0;

	// Populate the initial display state.
	dsWindowsHDRDisplayManager_update(displayManager);
	return displayManager;
}

bool dsWindowsHDRDisplayManager_update(dsWindowsHDRDisplayManager* displayManager)
{
	if (!displayManager)
	{
		errno = EINVAL;
		return false;
	}

	// NOTE: Need to create a new factory each time to get the updated display info.
	IDXGIFactory1* dxgiFactory;
	if (CreateDXGIFactory1(&dxgiFactory1ID, (void**)&dxgiFactory) != S_OK)
	{
		errno = EPERM;
		return false;
	}

	// First, iterate over the DXGI outputs, which are present on each adapter, to get the max
	// luminance values for each display.
	bool error = false;
	displayManager->displayCount = 0;
	UINT adapterIndex = 0;
	do
	{
		IDXGIAdapter* adapter;
		if (IDXGIFactory1_EnumAdapters(dxgiFactory, adapterIndex++, &adapter) != S_OK)
			break;

		UINT outputIndex = 0;
		do
		{
			IDXGIOutput* output;
			if (IDXGIAdapter_EnumOutputs(adapter, outputIndex++, &output) != S_OK)
				break;

			IDXGIOutput6* output6;
			HRESULT result = IDXGIOutput_QueryInterface(output, &dxgiOutput6ID, (void**)&output6);
			IDXGIOutput_Release(output);
			if (result != S_OK)
			{
				errno = EPERM;
				error = true;
				break;
			}

			DXGI_OUTPUT_DESC1 outputDesc;
			result = IDXGIOutput6_GetDesc1(output6, &outputDesc);
			IDXGIOutput6_Release(output6);
			if (result != S_OK)
			{
				errno = EPERM;
				error = true;
				break;
			}

			uint32_t index;
			for (index = 0; index < displayManager->displayCount; ++index)
			{
				DisplayInfo* display = displayManager->displays + index;
				if (display->monitor == outputDesc.Monitor)
					break;
			}

			if (index < displayManager->displayCount)
			{
				DisplayInfo* display = displayManager->displays + index;
				display->maxNits = dsMax(display->maxNits, outputDesc.MaxLuminance);
			}
			else
			{
				if (!DS_RESIZEABLE_ARRAY_ADD(displayManager->allocator, displayManager->displays,
						displayManager->displayCount, displayManager->maxDisplays, 1))
				{
					error = true;
					break;
				}

				DisplayInfo* display = displayManager->displays + index;
				display->monitor = outputDesc.Monitor;
				display->referenceNits = display->maxNits = outputDesc.MaxLuminance;;
				_Static_assert(sizeof(display->deviceName) == sizeof(outputDesc.DeviceName),
					"Unexpected display device name size.");
				memcpy(display->deviceName, outputDesc.DeviceName, sizeof(outputDesc.DeviceName));
			}
		} while (!error);

		IDXGIAdapter1_Release(adapter);
	} while (!error);

	IDXGIFactory1_Release(dxgiFactory);
	if (error)
		return false;

	// Then iterate over the display configs to get the reference white levels.
	uint32_t queryFlags = QDC_ONLY_ACTIVE_PATHS;
	uint32_t pathCount = 0;
	uint32_t modeCount = 0;
	uint32_t expectedPathCount, expectedModeCount;
	if (GetDisplayConfigBufferSizes(
			queryFlags, &expectedPathCount, &expectedModeCount) != ERROR_SUCCESS)
	{
		errno = EPERM;
		return false;
	}

	if (!DS_RESIZEABLE_ARRAY_ADD(displayManager->allocator, displayManager->tempPaths, pathCount,
			displayManager->maxTempPaths, expectedPathCount) ||
		!DS_RESIZEABLE_ARRAY_ADD(displayManager->allocator, displayManager->tempModes, modeCount,
			displayManager->maxTempModes, expectedModeCount))
	{
		return false;
	}

	do
	{
		HRESULT result = QueryDisplayConfig(queryFlags, &expectedPathCount,
			displayManager->tempPaths, &expectedModeCount, displayManager->tempModes, NULL);
		if (result == ERROR_SUCCESS)
			break;
		else if (result != ERROR_INSUFFICIENT_BUFFER)
		{
			errno = EPERM;
			return false;
		}

		// Initial estimates weren't sufficient.
		pathCount = modeCount = 0;
		if (!DS_RESIZEABLE_ARRAY_ADD(displayManager->allocator, displayManager->tempPaths,
				pathCount, displayManager->maxTempPaths, expectedPathCount) ||
			!DS_RESIZEABLE_ARRAY_ADD(displayManager->allocator, displayManager->tempModes,
				modeCount, displayManager->maxTempModes, expectedModeCount))
		{
			return false;
		}
	} while (true);

	for (uint32_t i = 0; i < pathCount; ++i)
	{
		const DISPLAYCONFIG_PATH_INFO* path = displayManager->tempPaths + i;

		// Must batch the display based on the source name.
		DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName;
		sourceName.header.adapterId = path->sourceInfo.adapterId;
		sourceName.header.id = path->sourceInfo.id;
		sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
		sourceName.header.size = sizeof(sourceName);
		if (DisplayConfigGetDeviceInfo(&sourceName.header) != ERROR_SUCCESS)
		{
			errno = EPERM;
			return false;
		}

		DisplayInfo* display = NULL;
		for (uint32_t j = 0; j < displayManager->displayCount; ++j)
		{
			DisplayInfo* thisDisplay = displayManager->displays + j;
			if (wcscmp(thisDisplay->deviceName, sourceName.viewGdiDeviceName) == 0)
			{
				display = thisDisplay;
				break;
			}
		}
		if (!display)
			continue;

		// Try to get the white level, which is based on the target.
		DISPLAYCONFIG_SDR_WHITE_LEVEL whiteLevel;
		whiteLevel.header.adapterId = path->targetInfo.adapterId;
		whiteLevel.header.id = path->targetInfo.id;
		whiteLevel.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL;
		whiteLevel.header.size = sizeof(whiteLevel);

		if (DisplayConfigGetDeviceInfo(&whiteLevel.header) != ERROR_SUCCESS ||
			whiteLevel.SDRWhiteLevel == 0)
		{
			continue;
		}

		display->referenceNits = (float)whiteLevel.SDRWhiteLevel*80.0f/1000.0f;
	}

	return true;
}

bool dsWindowsHDRDisplayManager_getDisplayLuminances(float* outReferenceNits, float* outMaxNits,
	const dsWindowsHDRDisplayManager* displayManager, const void* monitor)
{
	if (!outReferenceNits || !outMaxNits || !displayManager || !monitor)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < displayManager->displayCount; ++i)
	{
		const DisplayInfo* display = displayManager->displays + i;
		if (display->monitor == monitor)
		{
			*outReferenceNits = display->referenceNits;
			*outMaxNits = display->maxNits;
			return true;
		}
	}

	errno = ENOTFOUND;
	return false;
}

void dsWindowsHDRDisplayManager_destroy(dsWindowsHDRDisplayManager* displayManager)
{
	if (!displayManager)
		return;

	DS_VERIFY(dsAllocator_free(displayManager->allocator, displayManager->displays));
	DS_VERIFY(dsAllocator_free(displayManager->allocator, displayManager->tempPaths));
	DS_VERIFY(dsAllocator_free(displayManager->allocator, displayManager->tempModes));
	DS_VERIFY(dsAllocator_free(displayManager->allocator, displayManager));
}

#endif // DS_WINDOWS
