if (NOT TARGET DeepSea::VectorDrawScene OR NOT TARGET DeepSea::ApplicationSDL)
	return()
endif()

if (NOT MSLC)
	message("mslc shader compiler not found, skipping TestVectorDrawScene.")
	return()
endif()

set(shaderConfigs)
set(source TestVectorDrawScene.c Text.msl VectorElementFill.msl VectorElementImage.msl
	VectorElementLine.msl VectorElementTextColor.msl VectorElementTextGradient.msl)
if (TARGET DeepSea::RenderOpenGL)
	list(APPEND shaderConfigs glsl-1.1 glsl-1.5 glsl-es-1.0 glsl-es-3.0)
endif()
if (TARGET DeepSea::RenderVulkan)
	list(APPEND shaderConfigs spirv-1.0)
endif()
if (TARGET DeepSea::RenderMetal)
	if (IOS)
		list(APPEND shaderConfigs metal-ios-1.1)
	else()
		list(APPEND shaderConfigs metal-macos-1.1)
	endif()
endif()

if (NOT shaderConfigs)
	return()
endif()

ds_add_executable(deepsea_test_vector_draw_scene_app WIN32 ${source})
target_link_libraries(deepsea_test_vector_draw_scene_app 
	PRIVATE DeepSea::RenderBootstrap DeepSea::VectorDrawScene DeepSea::ApplicationSDL)
target_include_directories(deepsea_test_vector_draw_scene_app PRIVATE ${FLATBUFFERS_INCLUDE_DIRS})
ds_link_main_lib(deepsea_test_vector_draw_scene_app DeepSea::ApplicationSDLMain)

if (TARGET DeepSea::EasyProfiler)
	target_link_libraries(deepsea_test_vector_draw_scene_app PRIVATE DeepSea::EasyProfiler)
	target_compile_definitions(deepsea_test_vector_draw_scene_app PRIVATE DS_HAS_EASY_PROFILER=1)
endif()

ds_config_binary_dir(assetsDir assets)
ds_config_binary_dir(configDir)
set(shaderDir ${assetsDir}/shaders)
add_custom_target(deepsea_test_vector_draw_scene_app_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir})

# Compile shaders.
set(srcDir ${CMAKE_CURRENT_SOURCE_DIR})
ds_compile_shaders(shaders FILE ${srcDir}/Text.msl
	OUTPUT Text.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${DEEPSEA_MODULE_DIR}/Render/Render/include ${DEEPSEA_MODULE_DIR}/Scene/Scene/include
		${DEEPSEA_MODULE_DIR}/Text/include)
ds_compile_shaders(shaders FILE ${srcDir}/VectorElementFill.msl
	${srcDir}/VectorElementImage.msl
	${srcDir}/VectorElementLine.msl
	${srcDir}/VectorElementTextColor.msl
	${srcDir}/VectorElementTextGradient.msl
	OUTPUT Vector.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${DEEPSEA_MODULE_DIR}/Render/Render/include ${DEEPSEA_MODULE_DIR}/Scene/Scene/include
		${DEEPSEA_MODULE_DIR}/Text/include ${DEEPSEA_MODULE_DIR}/VectorDraw/include)
ds_compile_shaders_target(deepsea_test_vector_draw_scene_app_shaders shaders
	DEPENDS deepsea_test_vector_draw_scene_app_prepare)

# Convert vector resources.
ds_convert_svg(svgs FILE ${srcDir}/Ghostscript_Tiger.svg OUTPUT ${assetsDir}/Ghostscript_Tiger.dsvi)
ds_convert_svg_target(deepsea_test_vector_draw_scene_app_svgs svgs
	DEPENDS deepsea_test_vector_draw_scene_app_prepare)

ds_create_vector_resources(vectorResources FILE ${srcDir}/VectorResources.json
	OUTPUT ${assetsDir}/VectorResources.dsvr)
ds_create_vector_resources_target(deepsea_test_vector_draw_scene_app_vector_resources
	vectorResources DEPENDS deepsea_test_vector_draw_scene_app_prepare)

# Convert scene.
ds_scene_resource_shader_module_config(textModule Text.mslb ${assetsDir} ${shaderDir}
	${shaderConfigs})
ds_scene_resource_shader_module_config(vectorModule Vector.mslb ${assetsDir} ${shaderDir}
	${shaderConfigs})
ds_configure_file(SceneResources.json ${configDir}/SceneResources.json CONFIG_VAR assetsDir
	textModule vectorModule @ONLY)

ds_create_scene_resources(resources FILE ${configDir}/SceneResources.json
	OUTPUT ${assetsDir}/SceneResources.dssr)
ds_create_scene_resources_target(deepsea_test_vector_draw_scene_app_scene_resources resources
	DEPENDS deepsea_test_vector_draw_scene_app_shaders deepsea_test_vector_draw_scene_app_svgs
		deepsea_test_vector_draw_scene_app_vector_resources)

ds_create_scene(scene FILE ${srcDir}/Scene.json OUTPUT ${assetsDir}/Scene.dss
	MODULE_DIRECTORIES ${srcDir}/python)
ds_create_scene_target(deepsea_test_vector_draw_scene_app_scene scene
	DEPENDS deepsea_test_vector_draw_scene_app_prepare)

ds_create_view(view FILE ${srcDir}/View.json OUTPUT ${assetsDir}/View.dsv)
ds_create_view_target(deepsea_test_vector_draw_scene_app_view view
	DEPENDS deepsea_test_vector_draw_scene_app_prepare)

# Copy
ds_build_assets_dir(finalAssetsDir deepsea_test_vector_draw_scene_app)
set(finalAssetsDir ${finalAssetsDir}/TestVectorDrawScene-assets)
add_custom_target(deepsea_test_vector_draw_scene_app_assets
	DEPENDS deepsea_test_vector_draw_scene_app_shaders
		deepsea_test_vector_draw_scene_app_scene_resources deepsea_test_vector_draw_scene_app_scene
		deepsea_test_vector_draw_scene_app_view
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${finalAssetsDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${finalAssetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${assetsDir} ${finalAssetsDir}
	COMMENT "Copying assets for TestVectorDrawScene")
add_dependencies(deepsea_test_vector_draw_scene_app deepsea_test_vector_draw_scene_app_assets)

ds_set_folder(deepsea_test_vector_draw_scene_app tests/gui)
ds_set_folder(deepsea_test_vector_draw_scene_app_prepare tests/gui/Resources)
ds_set_folder(deepsea_test_vector_draw_scene_app_shaders tests/gui/Resources)
ds_set_folder(deepsea_test_vector_draw_scene_app_svgs tests/gui/Resources)
ds_set_folder(deepsea_test_vector_draw_scene_app_vector_resources tests/gui/Resources)
ds_set_folder(deepsea_test_vector_draw_scene_app_scene_resources tests/gui/Resources)
ds_set_folder(deepsea_test_vector_draw_scene_app_scene tests/gui/Resources)
ds_set_folder(deepsea_test_vector_draw_scene_app_view tests/gui/Resources)
ds_set_folder(deepsea_test_vector_draw_scene_app_assets tests/gui/Resources)
