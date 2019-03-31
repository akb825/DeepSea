if (APPLE)
	message(STATUS "Skipping RenderVulkan for Apple platforms.")
	return()
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/msl-config/ConfigPaths.cmake)

file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
ds_add_library(deepsea_render_vulkan MODULE RenderVulkan FILES ${sources} DEPENDS deepsea_render)
ds_target_include_directories(deepsea_render_vulkan
	PRIVATE ${DEEPSEA_MODULE_DIR}/Render/RenderVulkan/external
	${DEEPSEA_MODULE_DIR}/Render/RenderVulkan/src)
ds_target_link_libraries(deepsea_render_vulkan PRIVATE msl_client)

if (UNIX AND NOT APPLE)
	find_package(X11 QUIET)
	find_package(Wayland QUIET)
endif()

if (X11_FOUND)
	ds_target_link_libraries(deepsea_render_vulkan PRIVATE ${X11_LIBRARIES})
	ds_target_include_directories(deepsea_render_vulkan PRIVATE ${X11_INCLUDE_DIR})
	ds_target_compile_definitions(deepsea_render_vulkan PRIVATE DS_VK_HAS_X11=1)
else()
	ds_target_compile_definitions(deepsea_render_vulkan PRIVATE DS_VK_HAS_X11=0)
endif()

if (WAYLAND_FOUND)
	ds_target_link_libraries(deepsea_render_vulkan PRIVATE ${WAYLAND_LIBRARIES})
	ds_target_include_directories(deepsea_render_vulkan PRIVATE ${WAYLAND_INCLUDE_DIR})
	ds_target_compile_definitions(deepsea_render_vulkan PRIVATE DS_VK_HAS_WAYLAND=1)
else()
	ds_target_compile_definitions(deepsea_render_vulkan PRIVATE DS_VK_HAS_WAYLAND=0)
endif()

if (ANDROID)
	ds_target_link_libraries(deepsea_render_vulkan PRIVATE android)
endif()

file(GLOB shaderConfigs msl-config/*)
ds_install_library(TARGET deepsea_render_vulkan MODULE RenderVulkan DEPENDS Render
	CONFIG_FILES ${shaderConfigs}
	CONFIG_LINES "include(\${CMAKE_CURRENT_LIST_DIR}/ConfigPaths.cmake)")
