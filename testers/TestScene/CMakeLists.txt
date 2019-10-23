if (NOT TARGET deepsea_scene OR NOT TARGET deepsea_application_sdl)
	return()
endif()

if (NOT MSLC)
	message("mslc shader compiler not found, skipping TestScene.")
	return()
endif()

set(shaderConfigs)
set(source TestScene.c LightData.c LightData.h TestScene.msl)
if (TARGET deepsea_render_opengl)
	list(APPEND shaderConfigs glsl-1.1 glsl-1.5 glsl-es-1.0 glsl-es-3.0)
endif()
if (TARGET deepsea_render_vulkan)
	list(APPEND shaderConfigs spirv-1.0)
endif()
if (TARGET deepsea_render_metal)
	if (IOS)
		list(APPEND shaderConfigs metal-ios-1.1)
	else()
		list(APPEND shaderConfigs metal-osx-1.1)
	endif()
endif()

if (NOT shaderConfigs)
	return()
endif()

ds_add_executable(deepsea_test_scene_app WIN32 ${source})
target_link_libraries(deepsea_test_scene_app PRIVATE
	deepsea_render_bootstrap deepsea_scene deepsea_application_sdl)
ds_link_main_lib(deepsea_test_scene_app deepsea_application_sdl_main)

if (TARGET deepsea_easy_profiler)
	target_link_libraries(deepsea_test_scene_app PRIVATE deepsea_easy_profiler)
	target_compile_definitions(deepsea_test_scene_app PRIVATE DS_HAS_EASY_PROFILER=1)
endif()

# Cannot use generator expressions for the custom commands.
set(shaderDir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
add_custom_target(deepsea_test_scene_app_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir})

ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/TestScene.msl
	OUTPUT TestScene.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${DEEPSEA_MODULE_DIR}/Render/include ${DEEPSEA_MODULE_DIR}/Scene/include)
ds_compile_shaders_target(deepsea_test_scene_app_shaders shaders
	DEPENDS deepsea_test_scene_app_prepare)

ds_build_assets_dir(assetsDir deepsea_test_scene_app)
set(assetsDir ${assetsDir}/TestScene-assets)
add_custom_target(deepsea_test_scene_app_assets
	DEPENDS deepsea_test_scene_app_shaders
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${shaderDir} ${assetsDir}
	COMMENT "Copying assets for TestScene")
add_dependencies(deepsea_test_scene_app deepsea_test_scene_app_assets)

ds_set_folder(deepsea_test_scene_app tests/gui)
ds_set_folder(deepsea_test_scene_app_prepare tests/gui/Resources)
ds_set_folder(deepsea_test_scene_app_shaders tests/gui/Resources)
ds_set_folder(deepsea_test_scene_app_assets tests/gui/Resources)
