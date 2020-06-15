if (NOT TARGET deepsea_scene OR NOT TARGET deepsea_application_sdl)
	return()
endif()

if (NOT MSLC)
	message("mslc shader compiler not found, skipping TestScene.")
	return()
endif()

set(shaderConfigs)
set(source TestScene.c LightData.cpp LightData.h LightData.fbs TestSceneCube.msl
	TestSceneGround.msl)
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
target_include_directories(deepsea_test_scene_app PRIVATE ${FLATBUFFERS_INCLUDE_DIRS})
ds_link_main_lib(deepsea_test_scene_app deepsea_application_sdl_main)

if (TARGET deepsea_easy_profiler)
	target_link_libraries(deepsea_test_scene_app PRIVATE deepsea_easy_profiler)
	target_compile_definitions(deepsea_test_scene_app PRIVATE DS_HAS_EASY_PROFILER=1)
endif()

ds_convert_flatbuffers(generatedFlatbuffers FILE ${CMAKE_CURRENT_SOURCE_DIR}/LightData.fbs
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	INCLUDE ${DEEPSEA_MODULE_DIR}/Scene/Scene/include
	PYTHON ${CMAKE_CURRENT_SOURCE_DIR}/python)
ds_convert_flatbuffers_target(deepsea_test_scene_app_flatbuffers generatedFlatbuffers)
add_dependencies(deepsea_test_scene_app deepsea_test_scene_app_flatbuffers)

ds_config_binary_dir(assetsDir assets)
ds_config_binary_dir(configDir)
set(shaderDir ${assetsDir}/shaders)
add_custom_target(deepsea_test_scene_app_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir})

# Compile shaders.
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/TestSceneCube.msl
	${CMAKE_CURRENT_SOURCE_DIR}/TestSceneGround.msl
	OUTPUT TestScene.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${DEEPSEA_MODULE_DIR}/Render/Render/include ${DEEPSEA_MODULE_DIR}/Scene/Scene/include)
ds_compile_shaders_target(deepsea_test_scene_app_shaders shaders
	DEPENDS deepsea_test_scene_app_prepare)

# Convert scene.
set(sharedDir ${DEEPSEA_SOURCE_DIR}/testers/Shared)
if (ANDROID)
	set(textureFormat ETC1)
elseif (IOS)
	set(textureFormat PVRTC1_RGB_4BPP)
else()
	set(textureFormat BC1_RGB)
endif()
set(textureDecoration UNorm)
set(srcDir ${CMAKE_CURRENT_SOURCE_DIR})
ds_scene_resource_shader_module_config(testSceneModule TestScene.mslb ${assetsDir} ${shaderDir}
	${shaderConfigs})
ds_configure_file(resources.json ${configDir}/resources.json CONFIG_VAR testSceneModule @ONLY)

ds_create_scene_resources(resources FILE ${configDir}/resources.json
	OUTPUT ${assetsDir}/resources.dssr DEPENDS ${sharedDir}/SimpleTexture.png)
ds_create_scene_resources_target(deepsea_test_scene_app_scene_resources resources
	DEPENDS deepsea_test_scene_app_shaders)

ds_create_scene(scene FILE ${CMAKE_CURRENT_SOURCE_DIR}/scene.json OUTPUT ${assetsDir}/scene.dss
	MODULE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/python EXTENSIONS TestSceneExtension)
ds_create_scene_target(deepsea_test_scene_app_scene scene DEPENDS deepsea_test_scene_app_prepare)

ds_create_view(view FILE ${CMAKE_CURRENT_SOURCE_DIR}/view.json OUTPUT ${assetsDir}/view.dsv)
ds_create_view_target(deepsea_test_scene_app_view view DEPENDS deepsea_test_scene_app_prepare)

# Copy
ds_build_assets_dir(finalAssetsDir deepsea_test_scene_app)
set(finalAssetsDir ${finalAssetsDir}/TestScene-assets)
add_custom_target(deepsea_test_scene_app_assets
	DEPENDS deepsea_test_scene_app_shaders deepsea_test_scene_app_scene_resources
		deepsea_test_scene_app_scene deepsea_test_scene_app_view
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${finalAssetsDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${finalAssetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${assetsDir} ${finalAssetsDir}
	COMMENT "Copying assets for TestScene")
add_dependencies(deepsea_test_scene_app deepsea_test_scene_app_assets)

ds_set_folder(deepsea_test_scene_app tests/gui)
ds_set_folder(deepsea_test_scene_app_flatbuffers tests/gui/Resources)
ds_set_folder(deepsea_test_scene_app_prepare tests/gui/Resources)
ds_set_folder(deepsea_test_scene_app_shaders tests/gui/Resources)
ds_set_folder(deepsea_test_scene_app_scene_resources tests/gui/Resources)
ds_set_folder(deepsea_test_scene_app_scene tests/gui/Resources)
ds_set_folder(deepsea_test_scene_app_view tests/gui/Resources)
ds_set_folder(deepsea_test_scene_app_assets tests/gui/Resources)
