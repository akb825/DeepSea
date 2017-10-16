if (NOT TARGET deepsea_application_sdl)
	return()
endif()

include(${DEEPSEA_SOURCE_DIR}/Render/cmake/CompileShaders.cmake)
if (NOT MSLC)
	message("mslc shader compiler not found, skipping TestCube.")
	return()
endif()

include(${DEEPSEA_SOURCE_DIR}/Render/cmake/ConvertTexture.cmake)
if (NOT CUTTLEFISH)
	message("cuttlefish texture converter not found, skipping TestCube.")
	return()
endif()

set(libraries)
set(defines)
set(shaderConfigs)
set(source TestCube.c TestCube.msl)
if (TARGET deepsea_render_opengl)
	list(APPEND source SetupOpenGL.c SetupOpenGL.h)
	list(APPEND libraries deepsea_render_opengl)
	list(APPEND defines DS_HAS_OPENGL=1)
	list(APPEND shaderConfigs glsl-1.1 glsl-es-1.0)
else()
	list(APPEND defines DS_HAS_OPENGL=0)
endif()

if (NOT libraries)
	return()
endif()

add_executable(deepsea_test_cube ${source})
target_link_libraries(deepsea_test_cube PRIVATE
	deepsea_application_sdl
	${libraries})
target_compile_definitions(deepsea_test_cube PRIVATE ${defines})

# Cannot use generator expressions for the custom commands.
set(shaderDir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
set(textureDir ${CMAKE_CURRENT_BINARY_DIR}/textues)
add_custom_target(deepsea_test_cube_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${textureDir})

ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/TestCube.msl
	OUTPUT TestCube.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir})
ds_compile_shaders_target(deepsea_test_cube_shaders shaders DEPENDS deepsea_test_cube_prepare)
add_dependencies(deepsea_test_cube deepsea_test_cube_shaders)

ds_convert_texture(textures IMAGE ${CMAKE_CURRENT_SOURCE_DIR}/texture.png FORMAT R8G8B8A8
	MIPMAP OUTPUT ${textureDir}/texture.pvr)
ds_convert_textures_target(deepsea_test_cube_textures textures DEPENDS deepsea_test_cube_prepare)
add_dependencies(deepsea_test_cube deepsea_test_cube_textures)

set(assetsDir $<TARGET_FILE_DIR:deepsea_test_cube>/TestCube-assets)
add_custom_target(deepsea_test_cube_assets ALL DEPENDS deepsea_test_cube
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${shaderDir} ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${textureDir} ${assetsDir}
	COMMENT "Copying assets for TestCube")

ds_set_folder(deepsea_test_cube tests)
ds_set_folder(deepsea_test_cube_prepare tests/Resources)
ds_set_folder(deepsea_test_cube_shaders tests/Resources)
ds_set_folder(deepsea_test_cube_textures tests/Resources)
ds_set_folder(deepsea_test_cube_assets tests/Resources)
