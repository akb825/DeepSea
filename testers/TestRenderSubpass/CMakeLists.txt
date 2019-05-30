if (NOT TARGET deepsea_application_sdl)
	return()
endif()

if (NOT MSLC)
	message("mslc shader compiler not found, skipping TestRenderSubpass.")
	return()
endif()

if (NOT CUTTLEFISH)
	message("cuttlefish texture converter not found, skipping TestRenderSubpass.")
	return()
endif()

set(shaderConfigs)
set(source TestRenderSubpass.c Cube.msl Resolve.msl)
if (TARGET deepsea_render_opengl)
	list(APPEND shaderConfigs glsl-1.1 glsl-1.5 glsl-es-1.0 glsl-es-3.0)
endif()
if (TARGET deepsea_render_vulkan)
	list(APPEND shaderConfigs spirv-1.0)
endif()
if (TARGET deepsea_render_metal)
	list(APPEND shaderConfigs metal-ios-1.1 metal-osx-1.1)
endif()

if (NOT shaderConfigs)
	return()
endif()

ds_add_executable(deepsea_test_render_subpass_app WIN32 ${source})
target_link_libraries(deepsea_test_render_subpass_app PRIVATE
	deepsea_render_bootstrap deepsea_application_sdl)
ds_link_main_lib(deepsea_test_render_subpass_app deepsea_application_sdl_main)

# Cannot use generator expressions for the custom commands.
set(shaderDir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
set(textureDir ${CMAKE_CURRENT_BINARY_DIR}/textues)
add_custom_target(deepsea_test_render_subpass_app_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${textureDir})

ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/Cube.msl
	${CMAKE_CURRENT_SOURCE_DIR}/Resolve.msl
	OUTPUT TestRenderSubpass.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${DEEPSEA_MODULE_DIR}/Render/include)
ds_compile_shaders_target(deepsea_test_render_subpass_app_shaders shaders
	DEPENDS deepsea_test_render_subpass_app_prepare)

ds_convert_texture(textures IMAGE ${CMAKE_CURRENT_SOURCE_DIR}/../Shared/SimpleTexture.png
	FORMAT R8G8B8A8 MIPMAP OUTPUT ${textureDir}/texture.pvr)
ds_convert_textures_target(deepsea_test_render_subpass_app_textures textures
	DEPENDS deepsea_test_render_subpass_app_prepare)

ds_build_assets_dir(assetsDir)
set(assetsDir ${assetsDir}/TestRenderSubpass-assets)
add_custom_target(deepsea_test_render_subpass_app_assets
	DEPENDS deepsea_test_render_subpass_app_shaders deepsea_test_render_subpass_app_textures
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${shaderDir} ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${textureDir} ${assetsDir}
	COMMENT "Copying assets for TestRenderSubpass")
add_dependencies(deepsea_test_render_subpass_app deepsea_test_render_subpass_app_assets)

ds_set_folder(deepsea_test_render_subpass_app tests/gui)
ds_set_folder(deepsea_test_render_subpass_app_prepare tests/gui/Resources)
ds_set_folder(deepsea_test_render_subpass_app_shaders tests/gui/Resources)
ds_set_folder(deepsea_test_render_subpass_app_textures tests/gui/Resources)
ds_set_folder(deepsea_test_render_subpass_app_assets tests/gui/Resources)
