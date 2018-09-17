if (APPLE)
	message("Skipping RenderVulkan for Apple platforms.")
	return()
endif()

file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
ds_add_library(deepsea_render_vulkan MODULE RenderVulkan FILES ${sources} DEPENDS deepsea_render)
ds_target_include_directories(deepsea_render_vulkan
	PRIVATE ${DEEPSEA_MODULE_DIR}/Render/RenderVulkan/external
	${DEEPSEA_MODULE_DIR}/Render/RenderVulkan/src)
ds_target_link_libraries(deepsea_render_vulkan PRIVATE msl_client)

ds_install_library(TARGET deepsea_render_vulkan MODULE RenderVulkan DEPENDS Render)
