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

#include <DeepSea/Application/Platform/WaylandColorManager.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

#include <string.h>

#if DS_APPLICATION_HAS_WAYLAND_COLOR_MANAGER

#include "Platform/WaylandLib.h"
#include "color-management-v1-protocol.h"
#include "wayland-protocol.h"
#include <poll.h>

#define MAX_EVENT_WAIT_MS 500

typedef struct OutputLuminances
{
	struct wl_output* output;
	struct wp_color_management_output_v1* colorManagementOutput;
	struct wp_image_description_v1* imageDescription;
	struct wp_image_description_info_v1* descriptionInfo;

	uint32_t minLuminance;
	uint32_t maxLuminance;
	uint32_t referenceLuminance;
} OutputLuminances;

struct dsWaylandColorManagerSurface
{
	dsWaylandColorManager* colorManager;
	struct wl_surface* surface;
	struct wl_output* output;
	struct wp_color_management_surface_v1* colorManagementSurface;
	struct wp_image_description_v1* imageDescription;

	dsRenderColorSpace colorSpace;
	uint32_t referenceLuminance;
	uint32_t maxLuminance;
};

struct dsWaylandColorManager
{
	dsAllocator* allocator;

	struct wl_display* display;
	struct wl_event_queue* globalEventQueue;
	struct wl_event_queue* colorManagerEventQueue;
	struct wl_event_queue* outputEventQueue;
	struct wl_event_queue* surfaceEventQueue;
	struct wl_registry* registry;
	struct wp_color_manager_v1* colorManager;

	bool expectColorManagerEvents;
	bool expectOutputEvents;
	bool expectSurfaceEvents;
	bool changedSurface;

	int displayFD;
	uint32_t featureMask;
	uint32_t primariesMask;
	uint32_t transferFunctionMask;

	OutputLuminances* outputLuminances;
	uint32_t outputLuminancesCount;
	uint32_t maxOutputLuminances;

	dsWaylandColorManagerSurface** surfaces;
	uint32_t surfaceCount;
	uint32_t maxSurfaces;
};

static void destroyOutputLuminances(OutputLuminances* outputLuminances)
{
	if (outputLuminances->colorManagementOutput)
		wp_color_management_output_v1_destroy(outputLuminances->colorManagementOutput);
	if (outputLuminances->imageDescription)
		wp_image_description_v1_destroy(outputLuminances->imageDescription);
	if (outputLuminances->descriptionInfo)
		wp_image_description_info_v1_destroy(outputLuminances->descriptionInfo);
}

static bool flushDisplay(struct wl_display* display, int displayFD, bool wait)
{
	struct pollfd pollFD = {displayFD, POLLOUT};
	do
	{
		if (wl_display_flush(display) >= 0)
			break;
		else if (!wait || errno != EAGAIN)
		{
			wl_display_cancel_read(display);
			return false;
		}

		// Wait until the file descriptor is writable if we're willing to wait.
		if (poll(&pollFD, 1, -1) < 0 && errno != EAGAIN && errno != EINTR)
		{
			wl_display_cancel_read(display);
			return false;
		}
	} while (true);
	return true;
}

static bool submitQueue(
	struct wl_display* display, int displayFD, struct wl_event_queue* queue, bool wait)
{
	// Prepare the queue for reading the latest events.
	bool didDispatch = false;
	do
	{
		if (wl_display_prepare_read_queue(display, queue) == 0)
			break;
		else if (errno != EAGAIN)
			return false;

		didDispatch = true;
		if (wl_display_dispatch_queue_pending(display, queue) < 0)
			return false;
	} while (true);

	// Flush pending display information to allow for events to be read.
	if (!flushDisplay(display, displayFD, wait))
		return false;

	// Read events if we can read from the file descriptor. If there were pending events to be
	// dispatched, assume they were queued from a prior flush and display read, in which case the
	// wait (if requested) will be skipped and lack of input data will be considered a success.
	struct pollfd pollFD = {displayFD, POLLIN};
	if (wait && !didDispatch)
	{
		do
		{
			int result = poll(&pollFD, 1, MAX_EVENT_WAIT_MS);
			if (result > 0)
				break;
			else if (result == 0 || (errno != EAGAIN && errno != EINTR))
			{
				wl_display_cancel_read(display);
				return false;
			}
		} while (true);
	}
	else if (poll(&pollFD, 1, 0) <= 0)
	{
		wl_display_cancel_read(display);
		// A previous flush may have dispatched events, in which case consider it a success.
		return didDispatch;
	}

	return wl_display_read_events(display) >= 0 &&
		wl_display_dispatch_queue_pending(display, queue) >= 0;
}

static void dsWaylandColorManagerSurface_imageDescriptionFailed(
	void* data, struct wp_image_description_v1* imageDescription, uint32_t cause,
	const char* msg)
{
	DS_UNUSED(data);
	DS_UNUSED(imageDescription);
	DS_UNUSED(cause);
	DS_LOG_ERROR_F(DS_APPLICATION_LOG_TAG, "Couldn't create Wayland image description: %s", msg);
	// Leave the image description on the surface so it doesn't get re-created each update.
}

static void dsWaylandColorManagerSurface_imageDescriptionReady2(void* data,
	struct wp_image_description_v1* imageDescription, uint32_t identityHi,
	uint32_t identityLo)
{
	DS_UNUSED(identityHi);
	DS_UNUSED(identityLo);
	dsWaylandColorManagerSurface* surface = (dsWaylandColorManagerSurface*)data;
	if (imageDescription == surface->imageDescription)
	{
		wp_color_management_surface_v1_set_image_description(surface->colorManagementSurface,
			imageDescription, WP_COLOR_MANAGER_V1_RENDER_INTENT_PERCEPTUAL);
	}
}

static void dsWaylandColorManagerSurface_imageDescriptionReady(
	void* data, struct wp_image_description_v1* imageDescription, uint32_t identity)
{
	dsWaylandColorManagerSurface_imageDescriptionReady2(data, imageDescription, 0, identity);
}

struct wp_image_description_v1_listener imageDescriptionListener =
{
	&dsWaylandColorManagerSurface_imageDescriptionFailed,
	&dsWaylandColorManagerSurface_imageDescriptionReady,
	&dsWaylandColorManagerSurface_imageDescriptionReady2
};

static void dsWaylandColorManagerSurface_setLuminances(
	dsWaylandColorManagerSurface* surface, uint32_t referenceLuminance, uint32_t maxLuminance)
{
	if (surface->referenceLuminance == referenceLuminance && surface->maxLuminance == maxLuminance)
		return;

	// Doesn't need to update the image description if not an HDR format and only the max luminance
	// changed.
	bool needsUpdate = surface->referenceLuminance != referenceLuminance ||
		surface->colorSpace >= dsRenderColorSpace_ExtendedLinearSRGB;
	if (needsUpdate && surface->imageDescription)
	{
		wp_image_description_v1_destroy(surface->imageDescription);
		surface->imageDescription = NULL;
	}
	surface->referenceLuminance = referenceLuminance;
	surface->maxLuminance = maxLuminance;;
	surface->colorManager->changedSurface = true;
}

static bool dsWaylandColorManagerSurface_updateImageDescription(
	dsWaylandColorManagerSurface* surface)
{
	if (surface->imageDescription || surface->referenceLuminance == 0 || surface->maxLuminance == 0)
		return true;

	dsWaylandColorManager* colorManager = surface->colorManager;
	struct wp_image_description_creator_params_v1* creatorParams =
		wp_color_manager_v1_create_parametric_creator(colorManager->colorManager);
	if (!creatorParams)
		return false;

	switch (surface->colorSpace)
	{
		case dsRenderColorSpace_NonLinearSRGB:
		case dsRenderColorSpace_NonLinearSRGBConverting:
			if (!(colorManager->transferFunctionMask &
					WP_COLOR_MANAGER_V1_TRANSFER_FUNCTION_SRGB) ||
				!(colorManager->primariesMask & WP_COLOR_MANAGER_V1_PRIMARIES_SRGB))
			{
				wp_image_description_creator_params_v1_destroy(creatorParams);
				errno = EPERM;
				return false;
			}

			wp_image_description_creator_params_v1_set_tf_named(
				creatorParams, WP_COLOR_MANAGER_V1_TRANSFER_FUNCTION_SRGB);
			wp_image_description_creator_params_v1_set_primaries_named(
				creatorParams, WP_COLOR_MANAGER_V1_PRIMARIES_SRGB);
			wp_image_description_creator_params_v1_set_luminances(
				creatorParams, 0, surface->referenceLuminance, surface->referenceLuminance);
			break;
		case dsRenderColorSpace_ExtendedLinearSRGB:
			if (!(colorManager->transferFunctionMask &
					WP_COLOR_MANAGER_V1_TRANSFER_FUNCTION_EXT_LINEAR) ||
				!(colorManager->primariesMask & WP_COLOR_MANAGER_V1_PRIMARIES_SRGB))
			{
				wp_image_description_creator_params_v1_destroy(creatorParams);
				errno = EPERM;
				return false;
			}

			wp_image_description_creator_params_v1_set_tf_named(
				creatorParams, WP_COLOR_MANAGER_V1_TRANSFER_FUNCTION_EXT_LINEAR);
			wp_image_description_creator_params_v1_set_primaries_named(
				creatorParams, WP_COLOR_MANAGER_V1_PRIMARIES_SRGB);
			wp_image_description_creator_params_v1_set_luminances(
				creatorParams, 0, surface->referenceLuminance, surface->maxLuminance);
			break;
		case dsRenderColorSpace_Rec2100PQ:
			if (!(colorManager->transferFunctionMask &
					WP_COLOR_MANAGER_V1_TRANSFER_FUNCTION_ST2084_PQ) ||
				!(colorManager->primariesMask & WP_COLOR_MANAGER_V1_PRIMARIES_BT2020))
			{
				wp_image_description_creator_params_v1_destroy(creatorParams);
				errno = EPERM;
				return false;
			}

			wp_image_description_creator_params_v1_set_tf_named(
				creatorParams, WP_COLOR_MANAGER_V1_TRANSFER_FUNCTION_ST2084_PQ);
			wp_image_description_creator_params_v1_set_primaries_named(
				creatorParams, WP_COLOR_MANAGER_V1_PRIMARIES_BT2020);
			// Transfer function defines the maximum luminance.
			wp_image_description_creator_params_v1_set_luminances(
				creatorParams, 0, surface->referenceLuminance, 10000);
			// Use the mastering luminance to define the actual max luminance to display.
			wp_image_description_creator_params_v1_set_mastering_luminance(
				creatorParams, 0, surface->maxLuminance);
			break;
		default:
			wp_image_description_creator_params_v1_destroy(creatorParams);
			errno = EINVAL;
			return false;
	}

	surface->imageDescription = wp_image_description_creator_params_v1_create(creatorParams);
	if (!surface->imageDescription)
		return false;

	wl_proxy_set_queue(
		(struct wl_proxy*)surface->imageDescription, colorManager->surfaceEventQueue);
	colorManager->expectSurfaceEvents = true;
	wp_image_description_v1_add_listener(
		surface->imageDescription, &imageDescriptionListener, surface);
	return true;
}

static void dsWaylandColorManagerSurface_destroyImpl(
	dsAllocator* allocator, dsWaylandColorManagerSurface* surface)
{
	wp_color_management_surface_v1_destroy(surface->colorManagementSurface);
	if (surface->imageDescription)
		wp_image_description_v1_destroy(surface->imageDescription);
	DS_VERIFY(dsAllocator_free(allocator, surface));
}

static void dsWaylandColorManager_imageDescriptionInfoDone(
	void* data, struct wp_image_description_info_v1* descriptionInfo)
{
	DS_UNUSED(data);
	DS_UNUSED(descriptionInfo);
}

static void dsWaylandColorManager_imageDescriptionICCFile(
	void* data, struct wp_image_description_info_v1* descriptionInfo, int32_t icc, uint32_t iccSize)
{
	DS_UNUSED(data);
	DS_UNUSED(descriptionInfo);
	DS_UNUSED(icc);
	DS_UNUSED(iccSize);
}

static void dsWaylandColorManager_imageDescriptionInfoPrimaries(void* data,
	struct wp_image_description_info_v1* descriptionInfo, int32_t rX, int32_t rY, int32_t gX,
	int32_t gY, int32_t bX, int32_t bY, int32_t wX, int32_t wY)
{
	DS_UNUSED(data);
	DS_UNUSED(descriptionInfo);
	DS_UNUSED(rX);
	DS_UNUSED(rY);
	DS_UNUSED(gX);
	DS_UNUSED(gY);
	DS_UNUSED(bX);
	DS_UNUSED(bY);
	DS_UNUSED(wX);
	DS_UNUSED(wY);
}

static void dsWaylandColorManager_imageDescriptionPrimariesNamed(
	void* data, struct wp_image_description_info_v1* descriptionInfo, uint32_t primaries)
{
	DS_UNUSED(data);
	DS_UNUSED(descriptionInfo);
	DS_UNUSED(primaries);
}

static void dsWaylandColorManager_imageDescriptionTransferPower(
	void* data, struct wp_image_description_info_v1* descriptionInfo, uint32_t eexp)
{
	DS_UNUSED(data);
	DS_UNUSED(descriptionInfo);
	DS_UNUSED(eexp);
}

static void dsWaylandColorManager_imageDescriptionTransferNamed(
	void* data, struct wp_image_description_info_v1* descriptionInfo, uint32_t tf)
{
	DS_UNUSED(data);
	DS_UNUSED(descriptionInfo);
	DS_UNUSED(tf);
}

static void dsWaylandColorManager_imageDescriptionLuminances(void* data,
	struct wp_image_description_info_v1* descriptionInfo, uint32_t minLuminance,
	uint32_t maxLuminance, uint32_t referenceLuminance)
{
	dsWaylandColorManager* colorManager = (dsWaylandColorManager*)data;
	for (uint32_t i = 0; i < colorManager->outputLuminancesCount; ++i)
	{
		OutputLuminances* outputLuminances = colorManager->outputLuminances + i;
		if (outputLuminances->descriptionInfo != descriptionInfo)
			continue;

		outputLuminances->minLuminance = minLuminance;
		outputLuminances->maxLuminance = maxLuminance;
		outputLuminances->referenceLuminance = referenceLuminance;

		// Update the luminance values for the surfaces that share the same surface.
		for (uint32_t j = 0; j < colorManager->surfaceCount; ++j)
		{
			dsWaylandColorManagerSurface* surface = colorManager->surfaces[j];
			if (surface->output == outputLuminances->output)
			{
				dsWaylandColorManagerSurface_setLuminances(
					surface, referenceLuminance, maxLuminance);
			}
		}
		break;
	}
}

static void dsWaylandColorManager_imageDescriptionInfoTargetPrimaries(void* data,
	struct wp_image_description_info_v1* descriptionInfo, int32_t rX, int32_t rY, int32_t gX,
	int32_t gY, int32_t bX, int32_t bY, int32_t wX, int32_t wY)
{
	DS_UNUSED(data);
	DS_UNUSED(descriptionInfo);
	DS_UNUSED(rX);
	DS_UNUSED(rY);
	DS_UNUSED(gX);
	DS_UNUSED(gY);
	DS_UNUSED(bX);
	DS_UNUSED(bY);
	DS_UNUSED(wX);
	DS_UNUSED(wY);
}

static void dsWaylandColorManager_imageDescriptionTargetLuminances(void* data,
	struct wp_image_description_info_v1* descriptionInfo, uint32_t minLuminance,
	uint32_t maxLuminance)
{
	DS_UNUSED(data);
	DS_UNUSED(descriptionInfo);
	DS_UNUSED(minLuminance);
	DS_UNUSED(maxLuminance);
}

static void dsWaylandColorManager_imageDescriptionTargetMaxContentLightLevel(void* data,
	struct wp_image_description_info_v1* descriptionInfo, uint32_t maxLightLevel)
{
	DS_UNUSED(data);
	DS_UNUSED(descriptionInfo);
	DS_UNUSED(maxLightLevel);
}

static void dsWaylandColorManager_imageDescriptionTargetMaxAverageLightLevel(void* data,
	struct wp_image_description_info_v1* descriptionInfo, uint32_t averageLightLevel)
{
	DS_UNUSED(data);
	DS_UNUSED(descriptionInfo);
	DS_UNUSED(averageLightLevel);
}

static struct wp_image_description_info_v1_listener imageDescriptionInfoListener =
{
	&dsWaylandColorManager_imageDescriptionInfoDone,
	&dsWaylandColorManager_imageDescriptionICCFile,
	&dsWaylandColorManager_imageDescriptionInfoPrimaries,
	&dsWaylandColorManager_imageDescriptionPrimariesNamed,
	&dsWaylandColorManager_imageDescriptionTransferPower,
	&dsWaylandColorManager_imageDescriptionTransferNamed,
	&dsWaylandColorManager_imageDescriptionLuminances,
	&dsWaylandColorManager_imageDescriptionInfoTargetPrimaries,
	&dsWaylandColorManager_imageDescriptionTargetLuminances,
	&dsWaylandColorManager_imageDescriptionTargetMaxContentLightLevel,
	&dsWaylandColorManager_imageDescriptionTargetMaxAverageLightLevel
};

static void dsWaylandColorManager_hookupOutputImageDescription(
	dsWaylandColorManager* colorManager, OutputLuminances* outputLuminances)
{
	DS_ASSERT(outputLuminances->colorManagementOutput);
	if (outputLuminances->imageDescription)
	{
		wp_image_description_v1_destroy(outputLuminances->imageDescription);
		outputLuminances->imageDescription = NULL;
	}

	if (outputLuminances->descriptionInfo)
	{
		wp_image_description_info_v1_destroy(outputLuminances->descriptionInfo);
		outputLuminances->descriptionInfo = NULL;
	}

	outputLuminances->imageDescription = wp_color_management_output_v1_get_image_description(
			outputLuminances->colorManagementOutput);
	if (!outputLuminances->imageDescription)
		return;

	outputLuminances->descriptionInfo = wp_image_description_v1_get_information(
		outputLuminances->imageDescription);
	if (!outputLuminances->descriptionInfo)
		return;

	wl_proxy_set_queue(
		(struct wl_proxy*)outputLuminances->descriptionInfo, colorManager->outputEventQueue);
	colorManager->expectOutputEvents = true;
	wp_image_description_info_v1_add_listener(
		outputLuminances->descriptionInfo, &imageDescriptionInfoListener, colorManager);
}

static void dsWaylandColorManager_imageDescriptionChanged(
	void* data, struct wp_color_management_output_v1* colorManagementOutput)
{
	dsWaylandColorManager* colorManager = (dsWaylandColorManager*)data;
	for (uint32_t i = 0; i < colorManager->outputLuminancesCount; ++i)
	{
		OutputLuminances* outputLuminances = colorManager->outputLuminances + i;
		if (outputLuminances->colorManagementOutput == colorManagementOutput)
		{
			dsWaylandColorManager_hookupOutputImageDescription(colorManager, outputLuminances);
			break;
		}
	}
}

static struct wp_color_management_output_v1_listener colorManagementOutputListener =
{
	&dsWaylandColorManager_imageDescriptionChanged
};

static void dsWaylandColorManager_hookupColorManagementOutput(
	dsWaylandColorManager* colorManager, OutputLuminances* outputLuminances)
{
	DS_ASSERT(!outputLuminances->colorManagementOutput);
	DS_ASSERT(colorManager->colorManager);
	outputLuminances->colorManagementOutput = wp_color_manager_v1_get_output(
		colorManager->colorManager, outputLuminances->output);
	if (!outputLuminances->colorManagementOutput)
		return;

	wl_proxy_set_queue((struct wl_proxy*)outputLuminances->colorManagementOutput,
		colorManager->colorManagerEventQueue);
	wp_color_management_output_v1_add_listener(
		outputLuminances->colorManagementOutput, &colorManagementOutputListener, colorManager);
	dsWaylandColorManager_hookupOutputImageDescription(colorManager, outputLuminances);
}

static void dsWaylandColorManager_colorManagerSupportedIntent(
	void* data, struct wp_color_manager_v1 *wpColorManager, uint32_t intent)
{
	DS_UNUSED(data);
	DS_UNUSED(wpColorManager);
	DS_UNUSED(intent);
}

static void dsWaylandColorManager_colorManagerSupportedFeature(
	void* data, struct wp_color_manager_v1 *wpColorManager, uint32_t feature)
{
	DS_UNUSED(wpColorManager);
	dsWaylandColorManager* colorManager = (dsWaylandColorManager*)data;
	colorManager->featureMask |= 1U << feature;
}

static void dsWaylandColorManager_colorManagerSupportedTransferFunction(
	void* data, struct wp_color_manager_v1 *wpColorManager, uint32_t transferFunction)
{
	DS_UNUSED(wpColorManager);
	dsWaylandColorManager* colorManager = (dsWaylandColorManager*)data;
	colorManager->transferFunctionMask |= 1U << transferFunction;
}

static void dsWaylandColorManager_colorManagerSupportedPrimaries(
	void* data, struct wp_color_manager_v1 *wpColorManager, uint32_t primaries)
{
	DS_UNUSED(wpColorManager);
	dsWaylandColorManager* colorManager = (dsWaylandColorManager*)data;
	colorManager->primariesMask |= 1U << primaries;
}

static void dsWaylandColorManager_colorManagerDone(
	void* data, struct wp_color_manager_v1 *wpColorManager)
{
	DS_UNUSED(data);
	DS_UNUSED(wpColorManager);
}

static struct wp_color_manager_v1_listener colorManagerListener =
{
	&dsWaylandColorManager_colorManagerSupportedIntent,
	&dsWaylandColorManager_colorManagerSupportedFeature,
	&dsWaylandColorManager_colorManagerSupportedTransferFunction,
	&dsWaylandColorManager_colorManagerSupportedPrimaries,
	&dsWaylandColorManager_colorManagerDone
};

static void dsWaylandColorManager_registryGlobal(void* data, struct wl_registry* registry,
	uint32_t name, const char* interface, uint32_t version)
{
	dsWaylandColorManager* colorManager = (dsWaylandColorManager*)data;
	if (strcmp(interface, wp_color_manager_v1_interface.name) == 0)
	{
		colorManager->colorManager = (struct wp_color_manager_v1*)wl_registry_bind(
			registry, name, &wp_color_manager_v1_interface, version);
		if (colorManager->colorManager)
		{
			wl_proxy_set_queue(
				(struct wl_proxy*)colorManager->colorManager, colorManager->colorManagerEventQueue);
			colorManager->expectColorManagerEvents = true;
			wp_color_manager_v1_add_listener(
				colorManager->colorManager, &colorManagerListener, colorManager);

			// Hook up the color manager to all the outputs that may have been added earlier.
			for (uint32_t i = 0; i < colorManager->outputLuminancesCount; ++i)
			{
				dsWaylandColorManager_hookupColorManagementOutput(
					colorManager, colorManager->outputLuminances + i);
			}
		}
	}
}

static void dsWaylandColorManager_registryGlobalRemove(
	void* data, struct wl_registry* registry, uint32_t name)
{
	DS_UNUSED(data);
	DS_UNUSED(registry);
	DS_UNUSED(name);
}

static struct wl_registry_listener registryListener =
{
	&dsWaylandColorManager_registryGlobal,
	&dsWaylandColorManager_registryGlobalRemove
};

dsWaylandColorManager* dsWaylandColorManager_create(
	dsAllocator* allocator, struct wl_display* display)
{
	if (!allocator || !display)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(
			DS_APPLICATION_LOG_TAG, "Wayland color manager allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	if (!dsWaylandLib_load())
		return NULL;

	dsWaylandColorManager* colorManager = DS_ALLOCATE_OBJECT(allocator, dsWaylandColorManager);
	if (!colorManager)
	{
		dsWaylandLib_unload();
		return NULL;
	}

	memset(colorManager, 0, sizeof(dsWaylandColorManager));
	colorManager->allocator = allocator;
	colorManager->display = display;
	colorManager->displayFD = wl_display_get_fd(display);

	colorManager->registry = wl_display_get_registry(display);
	if (!colorManager->registry)
	{
		dsWaylandColorManager_destroy(colorManager);
		return NULL;
	}

	colorManager->globalEventQueue = wl_display_create_queue(display);
	colorManager->colorManagerEventQueue = wl_display_create_queue(display);
	colorManager->outputEventQueue = wl_display_create_queue(display);
	colorManager->surfaceEventQueue = wl_display_create_queue(display);
	if (!colorManager->globalEventQueue || !colorManager->colorManagerEventQueue ||
		!colorManager->outputEventQueue || !colorManager->surfaceEventQueue)
	{
		dsWaylandColorManager_destroy(colorManager);
		return NULL;
	}

	wl_proxy_set_queue((struct wl_proxy*)colorManager->registry, colorManager->globalEventQueue);
	wl_registry_add_listener(colorManager->registry, &registryListener, colorManager);

	// Wait for all registry events before continuing. After this is done, we will know if a color
	// manager is available or not.
	if (!submitQueue(
			colorManager->display, colorManager->displayFD, colorManager->globalEventQueue, true))
	{
		dsWaylandColorManager_destroy(colorManager);
		return NULL;
	}

	if (!colorManager->colorManager)
	{
		dsWaylandColorManager_destroy(colorManager);
		errno = EPERM;
		return NULL;
	}

	// Submit all other pending events to ensure that we have an initial state. Only submit the
	// output event queue if we are reasonably sure that there are events to process.
	DS_ASSERT(colorManager->expectColorManagerEvents);
	if (submitQueue(colorManager->display, colorManager->displayFD,
			colorManager->colorManagerEventQueue, true))
	{
		submitQueue(colorManager->display, colorManager->displayFD,
				colorManager->outputEventQueue, colorManager->expectOutputEvents);
	}
	colorManager->expectColorManagerEvents = false;
	colorManager->expectOutputEvents = false;
	DS_ASSERT(!colorManager->expectSurfaceEvents); // No surfaces added yet.
	return colorManager;
}

bool dsWaylandColorManager_addOutput(dsWaylandColorManager* colorManager, struct wl_output* output)
{
	if (!colorManager)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < colorManager->outputLuminancesCount; ++i)
	{
		if (colorManager->outputLuminances[i].output == output)
		{
			DS_LOG_ERROR(DS_APPLICATION_LOG_TAG, "Wayland output already added to color manager.");
			errno = EPERM;
			return false;
		}
	}

	uint32_t index = colorManager->outputLuminancesCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(colorManager->allocator, colorManager->outputLuminances,
			colorManager->outputLuminancesCount, colorManager->maxOutputLuminances, 1))
	{
		return false;
	}

	OutputLuminances* outputLuminances = colorManager->outputLuminances + index;
	outputLuminances->output = output;
	outputLuminances->colorManagementOutput = NULL;
	outputLuminances->imageDescription = NULL;
	outputLuminances->descriptionInfo = NULL;
	outputLuminances->minLuminance = 0;
	outputLuminances->maxLuminance = 0;
	outputLuminances->referenceLuminance = 0;

	if (colorManager->colorManager)
		dsWaylandColorManager_hookupColorManagementOutput(colorManager, outputLuminances);
	return true;
}

bool dsWaylandColorManager_removeOutput(
	dsWaylandColorManager* colorManager, struct wl_output* output)
{
	if (!colorManager || !output)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < colorManager->outputLuminancesCount; ++i)
	{
		OutputLuminances* outputLuminances = colorManager->outputLuminances + i;
		if (outputLuminances->output != output)
			continue;

		destroyOutputLuminances(outputLuminances);
		// Constant-time removal since order doesn't matter.
		*outputLuminances = colorManager->outputLuminances[--colorManager->outputLuminancesCount];
		return true;
	}

	errno = ENOTFOUND;
	return false;
}

bool dsWaylandColorManager_getOutputLuminances(float* outReferenceNits,
	float* outMaxNits, const dsWaylandColorManager* colorManager, struct wl_output* output)
{
	if (!outReferenceNits || !outMaxNits || !colorManager || !output)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < colorManager->outputLuminancesCount; ++i)
	{
		OutputLuminances* outputLuminances = colorManager->outputLuminances + i;
		if (outputLuminances->output != output)
			continue;

		if (outputLuminances->referenceLuminance == 0 || outputLuminances->maxLuminance == 0)
		{
			errno = EAGAIN;
			return false;
		}

		*outReferenceNits = (float)outputLuminances->referenceLuminance;
		*outMaxNits = (float)outputLuminances->maxLuminance;
		return true;
	}

	errno = ENOTFOUND;
	return false;
}

bool dsWaylandColorManager_update(dsWaylandColorManager* colorManager)
{
	if (!colorManager)
		return false;

	colorManager->changedSurface = false;

	// Never wait on global events.
	submitQueue(
		colorManager->display, colorManager->displayFD, colorManager->globalEventQueue, false);

	// Wait on color manager and output events only if we expect there to be any.
	submitQueue(colorManager->display, colorManager->displayFD,
		colorManager->colorManagerEventQueue, colorManager->expectColorManagerEvents);
	colorManager->expectColorManagerEvents = false;

	submitQueue(colorManager->display, colorManager->displayFD,
		colorManager->outputEventQueue, colorManager->expectOutputEvents);
	colorManager->expectOutputEvents = false;

	// After the outputs are updated, ensure the image descriptions for each surface are up to date.
	// This ensures the image descrptions aren't created and piped through the protocol until we are
	// sure everything is in a steady state.
	for (uint32_t i = 0; i < colorManager->surfaceCount; ++i)
		dsWaylandColorManagerSurface_updateImageDescription(colorManager->surfaces[i]);
	if (submitQueue(colorManager->display, colorManager->displayFD,
			colorManager->surfaceEventQueue, colorManager->expectSurfaceEvents))
	{
		// If we had surface events, expect that image descriptions were set and need to be flushed.
		if (colorManager->expectSurfaceEvents)
			flushDisplay(colorManager->display, colorManager->displayFD, true);
	}
	colorManager->expectSurfaceEvents = false;

	return colorManager->changedSurface;
}

dsWaylandColorManagerSurface* dsWaylandColorManager_addSurface(dsWaylandColorManager* colorManager,
	struct wl_surface* surface, struct wl_output* output, dsRenderColorSpace colorSpace)
{
	if (!colorManager || !surface || colorSpace < dsRenderColorSpace_NonLinearSRGB ||
		colorSpace > dsRenderColorSpace_Rec2100PQ)
	{
		errno = EINVAL;
		return NULL;
	}

	for (uint32_t i = 0; i < colorManager->surfaceCount; ++i)
	{
		if (colorManager->surfaces[i]->surface == surface)
		{
			DS_LOG_ERROR(DS_APPLICATION_LOG_TAG, "Wayland surface already added to color manager.");
			errno = EPERM;
			return NULL;
		}
	}

	uint32_t referenceLuminance = 0;
	uint32_t maxLuminance = 0;
	if (output)
	{
		bool foundOutput = false;
		for (uint32_t i = 0; i < colorManager->outputLuminancesCount; ++i)
		{
			const OutputLuminances* outputLuminances = colorManager->outputLuminances + i;
			if (outputLuminances->output != output)
				continue;

			foundOutput = true;
			referenceLuminance = outputLuminances->referenceLuminance;
			maxLuminance = outputLuminances->maxLuminance;
			break;
		}

		if (!foundOutput)
		{
			DS_LOG_ERROR(DS_APPLICATION_LOG_TAG, "Wayland output not added to color manager.");
			errno = EPERM;
			return NULL;
		}
	}

	uint32_t index = colorManager->surfaceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(colorManager->allocator, colorManager->surfaces,
			colorManager->surfaceCount, colorManager->maxSurfaces, 1))
	{
		return NULL;
	}

	dsWaylandColorManagerSurface* colorManagerSurface = DS_ALLOCATE_OBJECT(
		colorManager->allocator, dsWaylandColorManagerSurface);
	if (!colorManagerSurface)
	{
		--colorManager->surfaceCount;
		return NULL;
	}

	colorManagerSurface->colorManager = colorManager;
	colorManagerSurface->surface = surface;
	colorManagerSurface->output = output;
	colorManagerSurface->colorManagementSurface = wp_color_manager_v1_get_surface(
		colorManager->colorManager, surface);
	if (!colorManagerSurface->colorManagementSurface)
	{
		--colorManager->surfaceCount;
		DS_VERIFY(dsAllocator_free(colorManager->allocator, colorManagerSurface));
		return NULL;
	}
	colorManagerSurface->imageDescription = NULL;
	colorManagerSurface->colorSpace = colorSpace;
	colorManagerSurface->referenceLuminance = referenceLuminance;
	colorManagerSurface->maxLuminance = maxLuminance;

	colorManager->surfaces[index] = colorManagerSurface;
	return colorManagerSurface;
}

void dsWaylandColorManager_destroy(dsWaylandColorManager* colorManager)
{
	if (!colorManager)
		return;

	if (colorManager->registry)
		wl_registry_destroy(colorManager->registry);
	if (colorManager->colorManager)
		wp_color_manager_v1_destroy(colorManager->colorManager);

	for (uint32_t i = 0; i < colorManager->outputLuminancesCount; ++i)
		destroyOutputLuminances(colorManager->outputLuminances + i);
	DS_VERIFY(dsAllocator_free(colorManager->allocator, colorManager->outputLuminances));

	for (uint32_t i = 0; i < colorManager->surfaceCount; ++i)
	{
		dsWaylandColorManagerSurface_destroyImpl(
			colorManager->allocator, colorManager->surfaces[i]);
	}
	DS_VERIFY(dsAllocator_free(colorManager->allocator, colorManager->surfaces));

	if (colorManager->globalEventQueue)
		wl_event_queue_destroy(colorManager->globalEventQueue);
	if (colorManager->colorManagerEventQueue)
		wl_event_queue_destroy(colorManager->colorManagerEventQueue);
	if (colorManager->outputEventQueue)
		wl_event_queue_destroy(colorManager->outputEventQueue);
	if (colorManager->surfaceEventQueue)
		wl_event_queue_destroy(colorManager->surfaceEventQueue);

	DS_VERIFY(dsAllocator_free(colorManager->allocator, colorManager));
	dsWaylandLib_unload();
}

bool dsWaylandColorManagerSurface_setColorSpace(
	dsWaylandColorManagerSurface* surface, dsRenderColorSpace colorSpace)
{
	if (!surface || colorSpace < dsRenderColorSpace_NonLinearSRGB ||
		colorSpace > dsRenderColorSpace_Rec2100PQ)
	{
		errno = EINVAL;
		return false;
	}

	if (surface->colorSpace == colorSpace)
		return true;

	if (surface->imageDescription)
	{
		wp_image_description_v1_destroy(surface->imageDescription);
		surface->imageDescription = NULL;
	}
	surface->colorSpace = colorSpace;
	return true;
}

bool dsWaylandColorManagerSurface_setOutput(
	dsWaylandColorManagerSurface* surface, struct wl_output* output)
{
	if (!surface)
	{
		errno = EINVAL;
		return false;
	}

	const dsWaylandColorManager* colorManager = surface->colorManager;
	uint32_t referenceLuminance = 0;
	uint32_t maxLuminance = 0;
	if (output)
	{
		bool foundOutput = false;
		for (uint32_t i = 0; i < colorManager->outputLuminancesCount; ++i)
		{
			const OutputLuminances* outputLuminances = colorManager->outputLuminances + i;
			if (outputLuminances->output != output)
				continue;

			foundOutput = true;
			referenceLuminance = outputLuminances->referenceLuminance;
			maxLuminance = outputLuminances->maxLuminance;
			break;
		}

		if (!foundOutput)
		{
			errno = EPERM;
			return false;
		}
	}

	dsWaylandColorManagerSurface_setLuminances(surface, referenceLuminance, maxLuminance);
	surface->output = output;
	return true;
}

bool dsWaylandColorManagerSurface_getLumiances(
	float* outReferenceNits, float* outMaxNits, const dsWaylandColorManagerSurface* surface)
{
	if (!outReferenceNits || !outMaxNits || !surface)
	{
		errno = EINVAL;
		return false;
	}

	*outReferenceNits = (float)surface->referenceLuminance;
	*outMaxNits = (float)surface->maxLuminance;
	return true;
}

void dsWaylandColorManagerSurface_destroy(dsWaylandColorManagerSurface* surface)
{
	if (!surface)
		return;

	dsWaylandColorManager* colorManager = surface->colorManager;
	uint32_t index;
	for (index = 0; index < colorManager->surfaceCount; ++index)
	{
		if (colorManager->surfaces[index] == surface)
			break;
	}

	DS_ASSERT(index < colorManager->surfaceCount);
	dsWaylandColorManagerSurface_destroyImpl(colorManager->allocator, surface);

	// Constant-time removal since order doesn't matter.
	colorManager->surfaces[index] = colorManager->surfaces[--colorManager->surfaceCount];
}

#endif // DS_APPLICATION_HAS_WAYLAND_COLOR_MANAGER
