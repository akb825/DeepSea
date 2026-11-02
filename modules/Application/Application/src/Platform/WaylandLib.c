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

#include "Platform/WaylandLib.h"

#include <DeepSea/Render/Types.h>
#include <DeepSea/Core/DynamicLib.h>
#include <DeepSea/Core/Log.h>

#if DS_APPLICATION_HAS_WAYLAND_COLOR_MANAGER

dsWlEventQueueDestroyFunction ds_wl_event_queue_destroy;
dsWLProxyMarshalFlagsFunction ds_wl_proxy_marshal_flags;
dsWLProxyMarshalArrayFlagsFunction ds_wl_proxy_marshal_array_flags;
dsWLProxyMarshalFunction ds_wl_proxy_marshal;
dsWLProxyMarshalArrayFunction ds_wl_proxy_marshal_array;
dsWLProxyCreateFunction ds_wl_proxy_create;
dsWLProxyCreateWrapperFunction ds_wl_proxy_create_wrapper;
dsWLProxyWrapperDestroyFunction ds_wl_proxy_wrapper_destroy;
dsWLProxyMarshalConstructionFunction ds_wl_proxy_marshal_constructor;
dsWLProxyMarshalConstructorVersionedFunction ds_wl_proxy_marshal_constructor_versioned;
dsWLProxyMarshalArrayConstructorFunction ds_wl_proxy_marshal_array_constructor;
dsWLProxyMarshalArrayConstructorVersionedFunction ds_wl_proxy_marshal_array_constructor_versioned;
dsWLProxyDestroyFunction ds_wl_proxy_destroy;
dsWLProxyAddListenerFunction ds_wl_proxy_add_listener;
dsWLProxySetUserDataFunction ds_wl_proxy_set_user_data;
dsWLProxyGetUserDataFunction ds_wl_proxy_get_user_data;
dsWLProxyGetVersionFunction ds_wl_proxy_get_version;
dsWLProxySetTagFunction ds_wl_proxy_set_tag;
dsWLProxyGetTagFunction ds_wl_proxy_get_tag;
dsWLProxySetQueueFunction ds_wl_proxy_set_queue;
dsWLDisplayGetFDFunction ds_wl_display_get_fd;
dsWLDisplayDispatchQueuePendingFunction ds_wl_display_dispatch_queue_pending;
dsWLDisplayFlushFunction ds_wl_display_flush;
dsWLDisplayCreateQueueFunction ds_wl_display_create_queue;
dsWLDisplayPrepareReadQueueFunction ds_wl_display_prepare_read_queue;
dsWLDisplayCancelReadFunction ds_wl_display_cancel_read;
dsWLDisplayReadEventsFunction ds_wl_display_read_events;

static dsDynamicLib wlClientLib;
static unsigned int wlLoadCount;

#define DS_LOAD_WAYLAND_FUNCTION(type, name) \
	do \
	{ \
		ds_ ## name = (type)dsDynamicLib_loadSymbol(&wlClientLib, #name); \
		if (!ds_ ## name) \
		{ \
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Couldn't load " #name); \
			dsWaylandLib_unload(); \
			return false; \
		} \
	} while (false)

bool dsWaylandLib_load(void)
{
	// NOTE: Assume this won't be done across threads.
	if (wlLoadCount++ > 0)
		return true;

	if (!dsDynamicLib_open(&wlClientLib, DS_LIBRARY_NAME("wayland-client")))
	{
		--wlLoadCount;
		return false;
	}

	DS_LOAD_WAYLAND_FUNCTION(dsWlEventQueueDestroyFunction, wl_event_queue_destroy);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyMarshalFlagsFunction, wl_proxy_marshal_flags);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyMarshalArrayFlagsFunction, wl_proxy_marshal_array_flags);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyMarshalFunction, wl_proxy_marshal);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyMarshalArrayFunction, wl_proxy_marshal_array);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyCreateFunction, wl_proxy_create);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyCreateWrapperFunction, wl_proxy_create_wrapper);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyWrapperDestroyFunction, wl_proxy_wrapper_destroy);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyMarshalConstructionFunction, wl_proxy_marshal_constructor);
	DS_LOAD_WAYLAND_FUNCTION(
		dsWLProxyMarshalConstructorVersionedFunction, wl_proxy_marshal_constructor_versioned);
	DS_LOAD_WAYLAND_FUNCTION(
		dsWLProxyMarshalArrayConstructorFunction, wl_proxy_marshal_array_constructor);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyMarshalArrayConstructorVersionedFunction,
		wl_proxy_marshal_array_constructor_versioned);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyDestroyFunction, wl_proxy_destroy);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyAddListenerFunction, wl_proxy_add_listener);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxySetUserDataFunction, wl_proxy_set_user_data);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyGetUserDataFunction, wl_proxy_get_user_data);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyGetVersionFunction, wl_proxy_get_version);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxySetTagFunction, wl_proxy_set_tag);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxyGetTagFunction, wl_proxy_get_tag);
	DS_LOAD_WAYLAND_FUNCTION(dsWLProxySetQueueFunction, wl_proxy_set_queue);
	DS_LOAD_WAYLAND_FUNCTION(dsWLDisplayGetFDFunction, wl_display_get_fd);
	DS_LOAD_WAYLAND_FUNCTION(
		dsWLDisplayDispatchQueuePendingFunction, wl_display_dispatch_queue_pending);
	DS_LOAD_WAYLAND_FUNCTION(dsWLDisplayFlushFunction, wl_display_flush);
	DS_LOAD_WAYLAND_FUNCTION(dsWLDisplayCreateQueueFunction, wl_display_create_queue);
	DS_LOAD_WAYLAND_FUNCTION(dsWLDisplayPrepareReadQueueFunction, wl_display_prepare_read_queue);
	DS_LOAD_WAYLAND_FUNCTION(dsWLDisplayCancelReadFunction, wl_display_cancel_read);
	DS_LOAD_WAYLAND_FUNCTION(dsWLDisplayReadEventsFunction, wl_display_read_events);
	return true;
}

void dsWaylandLib_unload(void)
{
	if (wlLoadCount == 0)
		return;

	if (--wlLoadCount == 0)
		dsDynamicLib_close(&wlClientLib);
}

#endif // DS_APPLICATION_HAS_WAYLAND_COLOR_MANAGER
