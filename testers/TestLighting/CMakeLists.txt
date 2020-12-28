if (NOT TARGET deepsea_scene_lighting OR NOT TARGET deepsea_application_sdl)
	return()
endif()

if (NOT MSLC)
	message("mslc shader compiler not found, skipping TestLighting.")
	return()
endif()

set(shaderConfigs)
set(source TestLighting.c ForwardLighting.msl Glass.msl sRGB.msl)
if (TARGET deepsea_render_opengl)
	list(APPEND shaderConfigs glsl-1.5 glsl-es-3.0)
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

ds_add_executable(deepsea_test_lighting_app WIN32 ${source})
target_link_libraries(deepsea_test_lighting_app PRIVATE
	deepsea_render_bootstrap deepsea_scene_lighting deepsea_application_sdl)
ds_link_main_lib(deepsea_test_lighting_app deepsea_application_sdl_main)

if (TARGET deepsea_easy_profiler)
	target_link_libraries(deepsea_test_lighting_app PRIVATE deepsea_easy_profiler)
	target_compile_definitions(deepsea_test_lighting_app PRIVATE DS_HAS_EASY_PROFILER=1)
endif()

ds_config_binary_dir(assetsDir assets)
ds_config_binary_dir(configDir)
set(shaderDir ${assetsDir}/shaders)
add_custom_target(deepsea_test_lighting_app_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir})

# Compile shaders.
set(shaderIncludes ${DEEPSEA_MODULE_DIR}/Render/Render/include
	${DEEPSEA_MODULE_DIR}/Scene/Scene/include ${DEEPSEA_MODULE_DIR}/Scene/SceneLighting/include)
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/ForwardLighting.msl
	OUTPUT ForwardLighting.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${shaderIncludes})
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/Glass.msl
	OUTPUT Glass.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${shaderIncludes})
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/sRGB.msl
	OUTPUT sRGB.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${shaderIncludes})
ds_compile_shaders_target(deepsea_test_lighting_app_shaders shaders
	DEPENDS deepsea_test_lighting_app_prepare)

# Variables used for configuring the scene resources.
if (ANDROID)
	set(rgbFormat ETC2_R8G8B8)
	set(normalFormat EAC_R11G11)
elseif (IOS)
	set(rgbFormat PVRTC1_RGB_4BPP)
	set(normalFormat EAC_R11G11)
else()
	set(rgbFormat BC1_RGB)
	set(normalFormat BC5)
endif()
set(srcDir ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB assetFiles Assets/*.png Assets/*.gltf)
ds_scene_resource_shader_module_config(forwardLightShaderModule ForwardLighting.mslb
	${assetsDir} ${shaderDir} ${shaderConfigs})
ds_scene_resource_shader_module_config(glassShaderModule Glass.mslb
	${assetsDir} ${shaderDir} ${shaderConfigs})
ds_scene_resource_shader_module_config(sRGBShaderModule sRGB.mslb
	${assetsDir} ${shaderDir} ${shaderConfigs})

# These resources need to be configured based on CMake variables.
ds_configure_file(Assets/BaseResources.json ${configDir}/BaseResources.json CONFIG_VAR
	glassShaderModule sRGBShaderModule @ONLY)
ds_configure_file(Assets/ForwardLightShaders.json ${configDir}/ForwardLightShaders.json CONFIG_VAR
	forwardLightShaderModule @ONLY)

# Convert the scene resources.
ds_create_scene_resources(resources FILE ${configDir}/BaseResources.json
	OUTPUT ${assetsDir}/BaseResources.dssr DEPENDS ${assetFiles})
ds_create_scene_resources(resources FILE ${srcDir}/Assets/Materials.json
	OUTPUT ${assetsDir}/Materials.dssr)
ds_create_scene_resources(resources FILE ${configDir}/ForwardLightShaders.json
	OUTPUT ${assetsDir}/ForwardLightShaders.dssr)
ds_create_scene_resources(resources FILE ${srcDir}/Assets/ForwardLightModels.json
	OUTPUT ${assetsDir}/ForwardLightModels.dssr)
ds_create_scene_resources(resources FILE ${srcDir}/Assets/SceneGraph.json
	OUTPUT ${assetsDir}/SceneGraph.dssr)
ds_create_scene_resources_target(deepsea_test_lighting_app_scene_resources resources
	DEPENDS deepsea_test_lighting_app_shaders)

# Convert the scenes and views.
ds_create_scene(scene FILE ${srcDir}/Assets/ForwardLightScene.json
	OUTPUT ${assetsDir}/ForwardLightScene.dss)
ds_create_scene_target(deepsea_test_lighting_app_scene scene
	DEPENDS deepsea_test_lighting_app_prepare)

ds_create_view(view FILE ${srcDir}/Assets/ForwardLightView.json
	OUTPUT ${assetsDir}/ForwardLightView.dsv)
ds_create_view_target(deepsea_test_lighting_app_view view DEPENDS deepsea_test_lighting_app_prepare)

# Copy
ds_build_assets_dir(finalAssetsDir deepsea_test_lighting_app)
set(finalAssetsDir ${finalAssetsDir}/TestLighting-assets)
add_custom_target(deepsea_test_lighting_app_assets
	DEPENDS deepsea_test_lighting_app_shaders deepsea_test_lighting_app_scene_resources
		deepsea_test_lighting_app_scene deepsea_test_lighting_app_view
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${finalAssetsDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${finalAssetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${assetsDir} ${finalAssetsDir}
	COMMENT "Copying assets for TestLighting")
add_dependencies(deepsea_test_lighting_app deepsea_test_lighting_app_assets)

ds_set_folder(deepsea_test_lighting_app tests/gui)
ds_set_folder(deepsea_test_lighting_app_prepare tests/gui/Resources)
ds_set_folder(deepsea_test_lighting_app_shaders tests/gui/Resources)
ds_set_folder(deepsea_test_lighting_app_scene_resources tests/gui/Resources)
ds_set_folder(deepsea_test_lighting_app_scene tests/gui/Resources)
ds_set_folder(deepsea_test_lighting_app_view tests/gui/Resources)
ds_set_folder(deepsea_test_lighting_app_assets tests/gui/Resources)
