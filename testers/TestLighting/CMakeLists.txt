if (NOT TARGET deepsea_scene_lighting OR NOT TARGET DeepSea::ApplicationSDL)
	return()
endif()

if (NOT MSLC)
	message("mslc shader compiler not found, skipping TestLighting.")
	return()
endif()

set(shaderConfigs)
set(fragmentInputShaderConfigs)
file(GLOB source "*.msl" "*.c" "*.h")
if (TARGET DeepSea::RenderOpenGL)
	list(APPEND shaderConfigs glsl-1.5 glsl-es-3.0)
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

ds_add_executable(deepsea_test_lighting_app WIN32 ${source})
target_link_libraries(deepsea_test_lighting_app
	PRIVATE DeepSea::RenderBootstrap DeepSea::SceneLighting DeepSea::ApplicationSDL)
ds_link_main_lib(deepsea_test_lighting_app DeepSea::ApplicationSDLMain)

if (TARGET DeepSea::EasyProfiler)
	target_link_libraries(deepsea_test_lighting_app PRIVATE DeepSea::EasyProfiler)
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
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/AmbientLight.msl
	${CMAKE_CURRENT_SOURCE_DIR}/DirectionalLight.msl ${CMAKE_CURRENT_SOURCE_DIR}/GBuffers.msl
	${CMAKE_CURRENT_SOURCE_DIR}/PointLight.msl ${CMAKE_CURRENT_SOURCE_DIR}/SpotLight.msl
	OUTPUT DeferredLighting.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${shaderIncludes})
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/ComputeSSAO.msl
	${CMAKE_CURRENT_SOURCE_DIR}/GBuffers.msl ${CMAKE_CURRENT_SOURCE_DIR}/SSAOAmbientLight.msl
	${CMAKE_CURRENT_SOURCE_DIR}/DirectionalLight.msl
	${CMAKE_CURRENT_SOURCE_DIR}/PointLight.msl ${CMAKE_CURRENT_SOURCE_DIR}/SpotLight.msl
	${CMAKE_CURRENT_SOURCE_DIR}/SSAOPrepass.msl
	OUTPUT SSAODeferredLighting.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${shaderIncludes} DEFINE HAS_PREPASS)
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/ForwardLighting.msl
	OUTPUT ForwardLighting.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${shaderIncludes})
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/ShadowCast.msl
	OUTPUT ShadowCast.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${shaderIncludes})
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/Glass.msl
	OUTPUT Glass.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${shaderIncludes})
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/sRGB.msl
	OUTPUT sRGB.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${shaderIncludes})

if (fragmentInputShaderConfigs)
	ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/AmbientLightFI.msl
		${CMAKE_CURRENT_SOURCE_DIR}/DirectionalLightFI.msl
		${CMAKE_CURRENT_SOURCE_DIR}/GBuffersFI.msl ${CMAKE_CURRENT_SOURCE_DIR}/PointLightFI.msl
		${CMAKE_CURRENT_SOURCE_DIR}/SpotLightFI.msl
		OUTPUT DeferredLightingFI.mslb CONFIG ${fragmentInputShaderConfigs} OUTPUT_DIR ${shaderDir}
		INCLUDE ${shaderIncludes})
	ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/ComputeSSAO.msl
		${CMAKE_CURRENT_SOURCE_DIR}/GBuffersFI.msl
		${CMAKE_CURRENT_SOURCE_DIR}/SSAOAmbientLightFI.msl
		${CMAKE_CURRENT_SOURCE_DIR}/DirectionalLightFI.msl
		${CMAKE_CURRENT_SOURCE_DIR}/PointLightFI.msl ${CMAKE_CURRENT_SOURCE_DIR}/SpotLightFI.msl
		${CMAKE_CURRENT_SOURCE_DIR}/SSAOPrepass.msl
		OUTPUT SSAODeferredLightingFI.mslb CONFIG ${fragmentInputShaderConfigs}
		OUTPUT_DIR ${shaderDir} INCLUDE ${shaderIncludes} DEFINE HAS_PREPASS)
	ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/ForwardLighting.msl
		OUTPUT ForwardLightingFI.mslb CONFIG ${fragmentInputShaderConfigs} OUTPUT_DIR ${shaderDir}
		INCLUDE ${shaderIncludes})
	ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/GlassDeferredFI.msl
		OUTPUT GlassDeferredFI.mslb CONFIG ${fragmentInputShaderConfigs} OUTPUT_DIR ${shaderDir}
		INCLUDE ${shaderIncludes})
	ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/Glass.msl
		OUTPUT GlassForwardFI.mslb CONFIG ${fragmentInputShaderConfigs} OUTPUT_DIR ${shaderDir}
		INCLUDE ${shaderIncludes})
	ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/sRGBDeferredFI.msl
		OUTPUT sRGBDeferredFI.mslb CONFIG ${fragmentInputShaderConfigs} OUTPUT_DIR ${shaderDir}
		INCLUDE ${shaderIncludes})
	ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/sRGBForwardFI.msl
		OUTPUT sRGBForwardFI.mslb CONFIG ${fragmentInputShaderConfigs} OUTPUT_DIR ${shaderDir}
		INCLUDE ${shaderIncludes})
endif()

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
ds_scene_resource_shader_module_config(deferredLightShaderModule DeferredLighting.mslb
	${assetsDir} ${shaderDir} ${shaderConfigs})
ds_scene_resource_shader_module_config(forwardLightShaderModule ForwardLighting.mslb
	${assetsDir} ${shaderDir} ${shaderConfigs})
ds_scene_resource_shader_module_config(shadowCastShaderModule ShadowCast.mslb
	${assetsDir} ${shaderDir} ${shaderConfigs})
ds_scene_resource_shader_module_config(glassShaderModule Glass.mslb
	${assetsDir} ${shaderDir} ${shaderConfigs})
ds_scene_resource_shader_module_config(sRGBShaderModule sRGB.mslb
	${assetsDir} ${shaderDir} ${shaderConfigs})
ds_scene_resource_shader_module_config(ssaoDeferredLightShaderModule SSAODeferredLighting.mslb
	${assetsDir} ${shaderDir} ${shaderConfigs})

if (fragmentInputShaderConfigs)
	ds_scene_resource_shader_module_config(deferredLightFIShaderModule DeferredLightingFI.mslb
		${assetsDir} ${shaderDir} ${fragmentInputShaderConfigs})
	ds_scene_resource_shader_module_config(forwardLightFIShaderModule ForwardLightingFI.mslb
		${assetsDir} ${shaderDir} ${fragmentInputShaderConfigs})
	ds_scene_resource_shader_module_config(glassDeferredFIShaderModule GlassDeferredFI.mslb
		${assetsDir} ${shaderDir} ${fragmentInputShaderConfigs})
	ds_scene_resource_shader_module_config(glassForwardFIShaderModule GlassForwardFI.mslb
		${assetsDir} ${shaderDir} ${fragmentInputShaderConfigs})
	ds_scene_resource_shader_module_config(sRGBDeferredFIShaderModule sRGBDeferredFI.mslb
		${assetsDir} ${shaderDir} ${fragmentInputShaderConfigs})
	ds_scene_resource_shader_module_config(sRGBForwardFIShaderModule sRGBForwardFI.mslb
		${assetsDir} ${shaderDir} ${fragmentInputShaderConfigs})
	ds_scene_resource_shader_module_config(ssaoDeferredLightFIShaderModule
		SSAODeferredLightingFI.mslb ${assetsDir} ${shaderDir} ${fragmentInputShaderConfigs})
endif()

# These resources need to be configured based on CMake variables.
ds_configure_file(Assets/BaseResources.json ${configDir}/BaseResources.json CONFIG_VAR
	shadowCastShaderModule @ONLY)
ds_configure_file(Assets/DeferredLightShaders.json ${configDir}/DeferredLightShaders.json CONFIG_VAR
	glassShaderModule sRGBShaderModule deferredLightShaderModule @ONLY)
ds_configure_file(Assets/ForwardLightShaders.json ${configDir}/ForwardLightShaders.json CONFIG_VAR
	glassShaderModule sRGBShaderModule forwardLightShaderModule @ONLY)
ds_configure_file(Assets/SSAOShaders.json ${configDir}/SSAOShaders.json
	CONFIG_VAR glassShaderModule sRGBShaderModule ssaoDeferredLightShaderModule @ONLY)

if (fragmentInputShaderConfigs)
	ds_configure_file(Assets/DeferredLightFIShaders.json ${configDir}/DeferredLightFIShaders.json
		CONFIG_VAR glassDeferredFIShaderModule sRGBDeferredFIShaderModule
		deferredLightFIShaderModule @ONLY)
	ds_configure_file(Assets/ForwardLightFIShaders.json ${configDir}/ForwardLightFIShaders.json
		CONFIG_VAR glassForwardFIShaderModule sRGBForwardFIShaderModule forwardLightFIShaderModule
		@ONLY)
	ds_configure_file(Assets/SSAOFIShaders.json ${configDir}/SSAOFIShaders.json
		CONFIG_VAR glassDeferredFIShaderModule sRGBDeferredFIShaderModule
		ssaoDeferredLightFIShaderModule @ONLY)
endif()

# Convert the scene resources.
ds_create_scene_resources(resources FILE ${configDir}/BaseResources.json
	OUTPUT ${assetsDir}/BaseResources.dssr DEPENDS ${assetFiles})
ds_create_scene_resources(resources FILE ${configDir}/DeferredLightShaders.json
	OUTPUT ${assetsDir}/DeferredLightShaders.dssr)
ds_create_scene_resources(resources FILE ${configDir}/ForwardLightShaders.json
	OUTPUT ${assetsDir}/ForwardLightShaders.dssr)
ds_create_scene_resources(resources FILE ${srcDir}/Assets/Materials.json
	OUTPUT ${assetsDir}/Materials.dssr)
ds_create_scene_resources(resources FILE ${srcDir}/Assets/Models.json
	OUTPUT ${assetsDir}/Models.dssr)
ds_create_scene_resources(resources FILE ${srcDir}/Assets/SceneGraph.json
	OUTPUT ${assetsDir}/SceneGraph.dssr)
ds_create_scene_resources(resources FILE ${configDir}/SSAOShaders.json
	OUTPUT ${assetsDir}/SSAOShaders.dssr)

if (fragmentInputShaderConfigs)
	ds_create_scene_resources(resources FILE ${configDir}/DeferredLightFIShaders.json
		OUTPUT ${assetsDir}/DeferredLightFIShaders.dssr)
	ds_create_scene_resources(resources FILE ${configDir}/ForwardLightFIShaders.json
		OUTPUT ${assetsDir}/ForwardLightFIShaders.dssr)
	ds_create_scene_resources(resources FILE ${configDir}/SSAOFIShaders.json
		OUTPUT ${assetsDir}/SSAOFIShaders.dssr)
endif()

ds_create_scene_resources_target(deepsea_test_lighting_app_scene_resources resources
	DEPENDS deepsea_test_lighting_app_shaders)

# Convert the scenes and views.
ds_create_scene(scene FILE ${srcDir}/Assets/DeferredLightScene.json
	OUTPUT ${assetsDir}/DeferredLightScene.dss)
ds_create_scene(scene FILE ${srcDir}/Assets/ForwardLightScene.json
	OUTPUT ${assetsDir}/ForwardLightScene.dss)
ds_create_scene(scene FILE ${srcDir}/Assets/SSAOScene.json OUTPUT ${assetsDir}/SSAOScene.dss)

if (fragmentInputShaderConfigs)
	ds_create_scene(scene FILE ${srcDir}/Assets/DeferredLightFIScene.json
		OUTPUT ${assetsDir}/DeferredLightFIScene.dss)
	ds_create_scene(scene FILE ${srcDir}/Assets/ForwardLightFIScene.json
		OUTPUT ${assetsDir}/ForwardLightFIScene.dss)
	ds_create_scene(scene FILE ${srcDir}/Assets/SSAOFIScene.json
		OUTPUT ${assetsDir}/SSAOFIScene.dss)
endif()

ds_create_scene_target(deepsea_test_lighting_app_scene scene
	DEPENDS deepsea_test_lighting_app_prepare)

ds_create_view(view FILE ${srcDir}/Assets/DeferredLightView.json
	OUTPUT ${assetsDir}/DeferredLightView.dsv)
ds_create_view(view FILE ${srcDir}/Assets/ForwardLightView.json
	OUTPUT ${assetsDir}/ForwardLightView.dsv)
ds_create_view(view FILE ${srcDir}/Assets/SSAOView.json OUTPUT ${assetsDir}/SSAOView.dsv)

if (fragmentInputShaderConfigs)
	ds_create_view(view FILE ${srcDir}/Assets/DeferredLightFIView.json
		OUTPUT ${assetsDir}/DeferredLightFIView.dsv)
	ds_create_view(view FILE ${srcDir}/Assets/ForwardLightFIView.json
		OUTPUT ${assetsDir}/ForwardLightFIView.dsv)
	ds_create_view(view FILE ${srcDir}/Assets/SSAOFIView.json OUTPUT ${assetsDir}/SSAOFIView.dsv)
endif()

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
