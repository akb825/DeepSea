if (NOT TARGET DeepSea::ApplicationSDL)
	return()
endif()

if (NOT MSLC)
	message("mslc shader compiler not found, skipping TestCube.")
	return()
endif()

if (NOT CUTTLEFISH)
	message("cuttlefish texture converter not found, skipping TestCube.")
	return()
endif()

set(shaderConfigs)
set(source TestCube.c TestCube.msl)
if (TARGET DeepSea::RenderOpenGL)
	list(APPEND shaderConfigs glsl-1.1 glsl-1.5 glsl-es-1.0)
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

ds_add_executable(deepsea_test_cube_app WIN32 ${source})
target_link_libraries(deepsea_test_cube_app
	PRIVATE DeepSea::RenderBootstrap DeepSea::ApplicationSDL)
ds_link_main_lib(deepsea_test_cube_app DeepSea::ApplicationSDLMain)

ds_config_binary_dir(shaderDir shaders)
set(textureDir ${CMAKE_CURRENT_BINARY_DIR}/textues)
add_custom_target(deepsea_test_cube_app_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${textureDir})

ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/TestCube.msl
	OUTPUT TestCube.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${DEEPSEA_MODULE_DIR}/Render/Render/include)
ds_compile_shaders_target(deepsea_test_cube_app_shaders shaders
	DEPENDS deepsea_test_cube_app_prepare)

ds_convert_texture(textures IMAGE ${CMAKE_CURRENT_SOURCE_DIR}/../Shared/SimpleTexture.png
	FORMAT R8G8B8A8 MIPMAP OUTPUT ${textureDir}/texture.pvr)
ds_convert_textures_target(deepsea_test_cube_app_textures textures
	DEPENDS deepsea_test_cube_app_prepare)

ds_build_assets_dir(assetsDir deepsea_test_cube_app)
set(assetsDir ${assetsDir}/TestCube-assets)
add_custom_target(deepsea_test_cube_app_assets
	DEPENDS deepsea_test_cube_app_shaders deepsea_test_cube_app_textures
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${shaderDir} ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${textureDir} ${assetsDir}
	COMMENT "Copying assets for TestCube")
add_dependencies(deepsea_test_cube_app deepsea_test_cube_app_assets)

ds_set_folder(deepsea_test_cube_app tests/gui)
ds_set_folder(deepsea_test_cube_app_prepare tests/gui/Resources)
ds_set_folder(deepsea_test_cube_app_shaders tests/gui/Resources)
ds_set_folder(deepsea_test_cube_app_textures tests/gui/Resources)
ds_set_folder(deepsea_test_cube_app_assets tests/gui/Resources)
