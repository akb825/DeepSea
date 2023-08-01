if (NOT TARGET DeepSea::SceneLighting OR NOT TARGET DeepSea::SceneAnimation OR NOT
	TARGET DeepSea::ApplicationSDL)
	return()
endif()

if (NOT MSLC)
	message("mslc shader compiler not found, skipping TestAnimation.")
	return()
endif()

set(shaderConfigs)
set(fragmentInputShaderConfigs)
file(GLOB source "*.msl" "*.c" "*.cpp" "*.h")
if (TARGET DeepSea::RenderOpenGL)
	list(APPEND shaderConfigs glsl-1.5 glsl-4.3 glsl-es-3.0 glsl-es-3.2)
endif()
if (TARGET DeepSea::RenderVulkan)
	list(APPEND shaderConfigs spirv-1.0)
endif()
if (TARGET DeepSea::RenderMetal)
	if (IOS)
		list(APPEND shaderConfigs metal-ios-1.1)
		list(APPEND fragmentInputShaderConfigs metal-ios-2.0)
	else()
		list(APPEND shaderConfigs metal-macos-1.1)
		list(APPEND fragmentInputShaderConfigs metal-macos-2.3)
	endif()
endif()

if (NOT shaderConfigs)
	return()
endif()

ds_add_executable(deepsea_test_animation_app WIN32 ${source})
target_link_libraries(deepsea_test_animation_app PRIVATE
	DeepSea::RenderBootstrap DeepSea::SceneAnimation DeepSea::SceneLighting DeepSea::ApplicationSDL)
target_include_directories(deepsea_test_animation_app PRIVATE ${FLATBUFFERS_INCLUDE_DIRS})
ds_link_main_lib(deepsea_test_animation_app DeepSea::ApplicationSDLMain)

if (TARGET DeepSea::EasyProfiler)
	target_link_libraries(deepsea_test_animation_app PRIVATE DeepSea::EasyProfiler)
	target_compile_definitions(deepsea_test_animation_app PRIVATE DS_HAS_EASY_PROFILER=1)
endif()

ds_config_binary_dir(assetsDir assets)
ds_config_binary_dir(configDir)
set(shaderDir ${assetsDir}/shaders)
add_custom_target(deepsea_test_animation_app_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir})

# Compile shaders.
set(shaderIncludes ${DEEPSEA_MODULE_DIR}/Animation/include
	${DEEPSEA_MODULE_DIR}/Render/Render/include ${DEEPSEA_MODULE_DIR}/Scene/Scene/include
	${DEEPSEA_MODULE_DIR}/Scene/SceneLighting/include
	${DEEPSEA_MODULE_DIR}/Scene/SceneAnimation/include)
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/Grass.msl
	${CMAKE_CURRENT_SOURCE_DIR}/Wood.msl ${CMAKE_CURRENT_SOURCE_DIR}/Pitch.msl
	${CMAKE_CURRENT_SOURCE_DIR}/Character.msl ${CMAKE_CURRENT_SOURCE_DIR}/ShadowCast.msl
	${CMAKE_CURRENT_SOURCE_DIR}/CharacterShadowCast.msl
	OUTPUT Shaders.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${shaderIncludes})

ds_compile_shaders_target(deepsea_test_animation_app_shaders shaders
	DEPENDS deepsea_test_animation_app_prepare)

# Variables used for configuring the scene resources.
if (ANDROID)
	set(rgbFormat ETC2_R8G8B8)
	set(rgbaFormat ETC2_R8G8B8A8)
	set(normalFormat EAC_R11G11)
elseif (IOS)
	set(rgbFormat PVRTC1_RGB_4BPP)
	set(rgbaFormat PVRTC1_RGBA_4BPP)
	set(normalFormat EAC_R11G11)
else()
	set(rgbFormat BC1_RGB)
	set(rgbaFormat BC3)
	set(normalFormat BC5)
endif()
set(srcDir ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB assetFiles Assets/*.png Assets/*.gltf)
ds_scene_resource_shader_module_config(shaderModule Shaders.mslb
	${assetsDir} ${shaderDir} ${shaderConfigs})

# These resources need to be configured based on CMake variables.
ds_configure_file(Assets/BaseResources.json ${configDir}/BaseResources.json CONFIG_VAR srcDir @ONLY)
ds_configure_file(Assets/Materials.json ${configDir}/Materials.json CONFIG_VAR shaderModule @ONLY)

# Convert the scene resources.
ds_create_scene_resources(resources FILE ${configDir}/BaseResources.json
	OUTPUT ${assetsDir}/BaseResources.dssr)
ds_create_scene_resources(resources FILE ${srcDir}/Assets/SkinBufferMaterials.json
	OUTPUT ${assetsDir}/SkinBufferMaterials.dssr DEPENDS ${assetFiles})
ds_create_scene_resources(resources FILE ${srcDir}/Assets/SkinTextureMaterials.json
	OUTPUT ${assetsDir}/SkinTextureMaterials.dssr DEPENDS ${assetFiles})
ds_create_scene_resources(resources FILE ${configDir}/Materials.json
	OUTPUT ${assetsDir}/Materials.dssr DEPENDS ${assetFiles})
ds_create_scene_resources(resources FILE ${srcDir}/Assets/SceneGraph.json
	OUTPUT ${assetsDir}/SceneGraph.dssr)

ds_create_scene_resources_target(deepsea_test_animation_app_scene_resources resources
	DEPENDS deepsea_test_animation_app_shaders)

# Convert the scene and view.
ds_create_scene(scene FILE ${srcDir}/Assets/Scene.json OUTPUT ${assetsDir}/Scene.dss
	MODULE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/python)
ds_create_scene_target(deepsea_test_animation_app_scene scene
	DEPENDS deepsea_test_animation_app_prepare)

ds_create_view(view FILE ${srcDir}/Assets/View.json OUTPUT ${assetsDir}/View.dsv)
ds_create_view_target(deepsea_test_animation_app_view view
	DEPENDS deepsea_test_animation_app_prepare)

# Copy
ds_build_assets_dir(finalAssetsDir deepsea_test_animation_app)
set(finalAssetsDir ${finalAssetsDir}/TestAnimation-assets)
add_custom_target(deepsea_test_animation_app_assets
	DEPENDS deepsea_test_animation_app_shaders deepsea_test_animation_app_scene_resources
		deepsea_test_animation_app_scene deepsea_test_animation_app_view
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${finalAssetsDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${finalAssetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${assetsDir} ${finalAssetsDir}
	COMMENT "Copying assets for TestAnimation")
add_dependencies(deepsea_test_animation_app deepsea_test_animation_app_assets)

ds_set_folder(deepsea_test_animation_app tests/gui)
ds_set_folder(deepsea_test_animation_app_prepare tests/gui/Resources)
ds_set_folder(deepsea_test_animation_app_shaders tests/gui/Resources)
ds_set_folder(deepsea_test_animation_app_scene_resources tests/gui/Resources)
ds_set_folder(deepsea_test_animation_app_scene tests/gui/Resources)
ds_set_folder(deepsea_test_animation_app_view tests/gui/Resources)
ds_set_folder(deepsea_test_animation_app_assets tests/gui/Resources)
