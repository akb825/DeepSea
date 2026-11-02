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

#include <DeepSea/Core/Config.h>
#include <stdbool.h>
#include <stdint.h>

#if DS_APPLICATION_HAS_WAYLAND_COLOR_MANAGER

#include <wayland-client-core.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Dynamically loaded functions for proxies and event queues to allow for Wayland to be an optional
// dependency.

typedef void (*dsWlEventQueueDestroyFunction)(struct wl_event_queue* queue);
typedef struct wl_proxy* (*dsWLProxyMarshalFlagsFunction)(struct wl_proxy* proxy, uint32_t opcode,
	const struct wl_interface* interface, uint32_t version, uint32_t flags, ...);
typedef struct wl_proxy* (*dsWLProxyMarshalArrayFlagsFunction)(struct wl_proxy* proxy,
	uint32_t opcode, const struct wl_interface* interface, uint32_t version, uint32_t flags,
	union wl_argument* args);
typedef void (*dsWLProxyMarshalFunction)(struct wl_proxy* p, uint32_t opcode, ...);
typedef void (*dsWLProxyMarshalArrayFunction)(
	struct wl_proxy* p, uint32_t opcode, union wl_argument* args);
typedef struct wl_proxy* (*dsWLProxyCreateFunction)(
	struct wl_proxy* factory, const struct wl_interface* interface);
typedef void* (*dsWLProxyCreateWrapperFunction)(void* proxy);
typedef void (*dsWLProxyWrapperDestroyFunction)(void* proxy_wrapper);
typedef struct wl_proxy* (*dsWLProxyMarshalConstructionFunction)(
	struct wl_proxy* proxy, uint32_t opcode, const struct wl_interface* interface, ...);
typedef struct wl_proxy* (*dsWLProxyMarshalConstructorVersionedFunction)(struct wl_proxy* proxy,
	uint32_t opcode, const struct wl_interface* interface, uint32_t version, ...);
typedef struct wl_proxy* (*dsWLProxyMarshalArrayConstructorFunction)(struct wl_proxy* proxy,
	uint32_t opcode, union wl_argument* args, const struct wl_interface* interface);
typedef struct wl_proxy* (*dsWLProxyMarshalArrayConstructorVersionedFunction)(
	struct wl_proxy* proxy, uint32_t opcode, union wl_argument* args,
	const struct wl_interface* interface, uint32_t version);
typedef void (*dsWLProxyDestroyFunction)(struct wl_proxy* proxy);
typedef int (*dsWLProxyAddListenerFunction)(
	struct wl_proxy* proxy, void (**implementation)(void), void* data);
typedef void (*dsWLProxySetUserDataFunction)(struct wl_proxy* proxy, void* user_data);
typedef void* (*dsWLProxyGetUserDataFunction)(struct wl_proxy* proxy);
typedef uint32_t (*dsWLProxyGetVersionFunction)(struct wl_proxy* proxy);
typedef void (*dsWLProxySetTagFunction)(struct wl_proxy* proxy, const char* const* tag);
typedef const char* const* (*dsWLProxyGetTagFunction)(struct wl_proxy* proxy);
typedef void (*dsWLProxySetQueueFunction)(struct wl_proxy* proxy, struct wl_event_queue* queue);
typedef int (*dsWLDisplayGetFDFunction)(struct wl_display* display);
typedef int (*dsWLDisplayDispatchQueuePendingFunction)(
	struct wl_display* display, struct wl_event_queue* queue);
typedef int (*dsWLDisplayFlushFunction)(struct wl_display* display);
typedef struct wl_event_queue* (*dsWLDisplayCreateQueueFunction)(struct wl_display* display);
typedef int (*dsWLDisplayPrepareReadQueueFunction)(
	struct wl_display* display, struct wl_event_queue* queue);
typedef void (*dsWLDisplayCancelReadFunction)(struct wl_display* display);
typedef int (*dsWLDisplayReadEventsFunction)(struct wl_display* display);

extern dsWlEventQueueDestroyFunction ds_wl_event_queue_destroy;
extern dsWLProxyMarshalFlagsFunction ds_wl_proxy_marshal_flags;
extern dsWLProxyMarshalArrayFlagsFunction ds_wl_proxy_marshal_array_flags;
extern dsWLProxyMarshalFunction ds_wl_proxy_marshal;
extern dsWLProxyMarshalArrayFunction ds_wl_proxy_marshal_array;
extern dsWLProxyCreateFunction ds_wl_proxy_create;
extern dsWLProxyCreateWrapperFunction ds_wl_proxy_create_wrapper;
extern dsWLProxyWrapperDestroyFunction ds_wl_proxy_wrapper_destroy;
extern dsWLProxyMarshalConstructionFunction ds_wl_proxy_marshal_constructor;
extern dsWLProxyMarshalConstructorVersionedFunction ds_wl_proxy_marshal_constructor_versioned;
extern dsWLProxyMarshalArrayConstructorFunction ds_wl_proxy_marshal_array_constructor;
extern dsWLProxyMarshalArrayConstructorVersionedFunction
	ds_wl_proxy_marshal_array_constructor_versioned;
extern dsWLProxyDestroyFunction ds_wl_proxy_destroy;
extern dsWLProxyAddListenerFunction ds_wl_proxy_add_listener;
extern dsWLProxySetUserDataFunction ds_wl_proxy_set_user_data;
extern dsWLProxyGetUserDataFunction ds_wl_proxy_get_user_data;
extern dsWLProxyGetVersionFunction ds_wl_proxy_get_version;
extern dsWLProxySetTagFunction ds_wl_proxy_set_tag;
extern dsWLProxyGetTagFunction ds_wl_proxy_get_tag;
extern dsWLProxySetQueueFunction ds_wl_proxy_set_queue;
extern dsWLDisplayGetFDFunction ds_wl_display_get_fd;
extern dsWLDisplayDispatchQueuePendingFunction ds_wl_display_dispatch_queue_pending;
extern dsWLDisplayFlushFunction ds_wl_display_flush;
extern dsWLDisplayCreateQueueFunction ds_wl_display_create_queue;
extern dsWLDisplayPrepareReadQueueFunction ds_wl_display_prepare_read_queue;
extern dsWLDisplayCancelReadFunction ds_wl_display_cancel_read;
extern dsWLDisplayReadEventsFunction ds_wl_display_read_events;

// Use defines to utilize dynamically loaded functions within generated protocol headers.
#define wl_event_queue_destroy ds_wl_event_queue_destroy
#define wl_proxy_marshal_flags ds_wl_proxy_marshal_flags
#define wl_proxy_marshal_array_flags ds_wl_proxy_marshal_array_flags
#define wl_proxy_marshal ds_wl_proxy_marshal
#define wl_proxy_marshal_array ds_wl_proxy_marshal_array
#define wl_proxy_create ds_wl_proxy_create
#define wl_proxy_create_wrapper ds_wl_proxy_create_wrapper
#define wl_proxy_wrapper_destroy ds_wl_proxy_wrapper_destroy
#define wl_proxy_marshal_constructor ds_wl_proxy_marshal_constructor
#define wl_proxy_marshal_constructor_versioned ds_wl_proxy_marshal_constructor_versioned
#define wl_proxy_marshal_array_constructor ds_wl_proxy_marshal_array_constructor
#define wl_proxy_marshal_array_constructor_versioned ds_wl_proxy_marshal_array_constructor_versioned
#define wl_proxy_destroy ds_wl_proxy_destroy
#define wl_proxy_add_listener ds_wl_proxy_add_listener
#define wl_proxy_set_user_data ds_wl_proxy_set_user_data
#define wl_proxy_get_user_data ds_wl_proxy_get_user_data
#define wl_proxy_get_version ds_wl_proxy_get_version
#define wl_proxy_set_tag ds_wl_proxy_set_tag
#define wl_proxy_get_tag ds_wl_proxy_get_tag
#define wl_proxy_set_queue ds_wl_proxy_set_queue
#define wl_display_get_fd ds_wl_display_get_fd
#define wl_display_dispatch_queue_pending ds_wl_display_dispatch_queue_pending
#define wl_display_flush ds_wl_display_flush
#define wl_display_create_queue ds_wl_display_create_queue
#define wl_display_prepare_read_queue ds_wl_display_prepare_read_queue
#define wl_display_cancel_read ds_wl_display_cancel_read
#define wl_display_read_events ds_wl_display_read_events

bool dsWaylandLib_load(void);
void dsWaylandLib_unload(void);

#ifdef __cplusplus
}
#endif

#endif // DS_APPLICATION_HAS_WAYLAND_COLOR_MANAGER
