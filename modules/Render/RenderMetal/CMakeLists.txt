if (NOT APPLE)
	message(STATUS "Skipping RenderMetal for non-Apple platforms.")
	return()
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/msl-config/ConfigPaths.cmake)

file(GLOB_RECURSE sources src/*.c src/*.m src/*.h include/*.h)
ds_add_library(deepsea_render_metal MODULE RenderMetal FILES ${sources})
ds_target_include_directories(deepsea_render_metal
	PRIVATE ${DEEPSEA_MODULE_DIR}/Render/RenderMetal/src)
ds_target_link_libraries(deepsea_render_metal PRIVATE MSL::Client "-framework Metal"
	"-framework QuartzCore" "-framework CoreFoundation")
if (IOS)
	ds_target_link_libraries(deepsea_render_metal PRIVATE "-framework Foundation"
		"-framework UIKit")
else()
	ds_target_link_libraries(deepsea_render_metal PRIVATE "-framework AppKit")
endif()

file(GLOB shaderConfigs msl-config/*)
ds_install_library(TARGET deepsea_render_metal MODULE RenderMetal DEPENDS Render
	CONFIG_FILES ${shaderConfigs}
	CONFIG_LINES "include(\${CMAKE_CURRENT_LIST_DIR}/ConfigPaths.cmake)")
