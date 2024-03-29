if (NOT TARGET DeepSea::ApplicationSDL OR NOT TARGET DeepSea::Text)
	return()
endif()

if (NOT MSLC)
	message("mslc shader compiler not found, skipping TestText.")
	return()
endif()

set(shaderConfigs)
set(source TestText.c Font.msl FontTess.msl Box.msl)
if (TARGET DeepSea::RenderOpenGL)
	list(APPEND shaderConfigs glsl-1.1 glsl-1.5 glsl-4.0 glsl-es-1.0 glsl-es-3.0 glsl-es-3.2)
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

ds_add_executable(deepsea_test_text_app WIN32 ${source})
target_link_libraries(deepsea_test_text_app
	PRIVATE DeepSea::RenderBootstrap DeepSea::ApplicationSDL DeepSea::Text)
ds_link_main_lib(deepsea_test_text_app DeepSea::ApplicationSDLMain)

if (TARGET DeepSea::EasyProfiler)
	target_link_libraries(deepsea_test_text_app PRIVATE DeepSea::EasyProfiler)
	target_compile_definitions(deepsea_test_text_app PRIVATE DS_HAS_EASY_PROFILER=1)
endif()

ds_config_binary_dir(shaderDir shaders)
add_custom_target(deepsea_test_text_app_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir})

ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/Font.msl
	${CMAKE_CURRENT_SOURCE_DIR}/FontTess.msl ${CMAKE_CURRENT_SOURCE_DIR}/Box.msl
	OUTPUT TestText.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${DEEPSEA_MODULE_DIR}/Render/Render/include ${DEEPSEA_MODULE_DIR}/Text/include)
ds_compile_shaders_target(deepsea_test_text_app_shaders shaders
	DEPENDS deepsea_test_text_app_prepare)

set(fontDir ${CMAKE_CURRENT_SOURCE_DIR}/../Shared/Fonts)

ds_build_assets_dir(assetsDir deepsea_test_text_app)
set(assetsDir ${assetsDir}/TestText-assets)
add_custom_target(deepsea_test_text_app_assets DEPENDS deepsea_test_text_app_shaders
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${shaderDir} ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${fontDir} ${assetsDir}/Fonts
	COMMENT "Copying assets for TestText")
add_dependencies(deepsea_test_text_app deepsea_test_text_app_assets)

ds_set_folder(deepsea_test_text_app tests/gui)
ds_set_folder(deepsea_test_text_app_prepare tests/gui/Resources)
ds_set_folder(deepsea_test_text_app_shaders tests/gui/Resources)
ds_set_folder(deepsea_test_text_app_assets tests/gui/Resources)
