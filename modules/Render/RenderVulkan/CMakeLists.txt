find_package(Vulkan)
if (NOT Vulkan_FOUND)
	message("Vulkan not found. Skipping RenderVulkan.")
	return()
endif()

file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
ds_add_library(deepsea_render_vulkan MODULE RenderVulkan FILES ${sources} DEPENDS deepsea_render)
ds_target_include_directories(deepsea_render_vulkan
	PRIVATE ${Vulkan_INCLUDE_DIRS} ${DEEPSEA_MODULE_DIR}/Render/RenderVulkan/src)
ds_target_link_libraries(deepsea_render_vulkan PRIVATE msl_client)

ds_install_library(TARGET deepsea_render_vulkan MODULE RenderVulkan DEPENDS Render)
