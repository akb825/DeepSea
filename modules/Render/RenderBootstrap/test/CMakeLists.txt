if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

set(shaderConfigs)
set(computeShaderConfigs)
if (TARGET deepsea_render_opengl)
	list(APPEND shaderConfigs glsl-1.1 glsl-es-1.0)
	list(APPEND bufferShaderConfigs glsl-1.5 glsl-es-3.0)
	list(APPEND computeShaderConfigs glsl-4.3 glsl-es-3.2)
endif()
if (TARGET deepsea_render_vulkan)
	list(APPEND computeShaderConfigs spirv-1.0)
endif()
if (TARGET deepsea_render_metal)
	if (IOS)
		list(APPEND computeShaderConfigs metal-ios-1.1)
	else()
		list(APPEND computeShaderConfigs metal-osx-1.1)
	endif()
endif()

if (NOT shaderConfigs AND NOT computeShaderConfigs)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h *.msl *.mslh)
ds_add_unittest(deepsea_render_bootstrap_test ${sources})

target_include_directories(deepsea_render_bootstrap_test PRIVATE .)
target_link_libraries(deepsea_render_bootstrap_test PRIVATE deepsea_render_bootstrap)

ds_config_binary_dir(shaderDir shaders)
add_custom_target(deepsea_render_bootstrap_test_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir})

set(shaderSourceDir ${CMAKE_CURRENT_SOURCE_DIR}/Shaders)
ds_compile_shaders(shaders FILE ${shaderSourceDir}/WriteOffscreen.msl
	OUTPUT WriteOffscreen.mslb CONFIG ${shaderConfigs} ${bufferShaderConfigs}
	${computeShaderConfigs} OUTPUT_DIR ${shaderDir} INCLUDE ${DEEPSEA_MODULE_DIR}/Render/Render/include
	DEPENDENCY_RECURSE ${DEEPSEA_MODULE_DIR}/Render/Render/include/*.mslh
	STRIP OPTIMIZE)
ds_compile_shaders(shaders FILE ${shaderSourceDir}/WriteOffscreenTransform.msl
	OUTPUT WriteOffscreenTransform.mslb CONFIG ${bufferShaderConfigs} ${computeShaderConfigs}
	OUTPUT_DIR ${shaderDir} INCLUDE ${DEEPSEA_MODULE_DIR}/Render/Render/include
	DEPENDENCY_RECURSE ${DEEPSEA_MODULE_DIR}/Render/Render/include/*.mslh STRIP OPTIMIZE)
ds_compile_shaders(shaders FILE ${shaderSourceDir}/WriteBuffer.msl
	OUTPUT WriteBuffer.mslb CONFIG ${computeShaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${DEEPSEA_MODULE_DIR}/Render/Render/include
	DEPENDENCY_RECURSE ${DEEPSEA_MODULE_DIR}/Render/Render/include/*.mslh STRIP OPTIMIZE)
ds_compile_shaders(shaders FILE ${shaderSourceDir}/CopyTextureBuffer.msl
	OUTPUT CopyTextureBuffer.mslb CONFIG ${computeShaderConfigs}
	OUTPUT_DIR ${shaderDir} INCLUDE ${DEEPSEA_MODULE_DIR}/Render/Render/include
	DEPENDENCY_RECURSE ${DEEPSEA_MODULE_DIR}/Render/Render/include/*.mslh STRIP OPTIMIZE)
ds_compile_shaders_target(deepsea_render_bootstrap_test_shaders shaders
	DEPENDS deepsea_render_bootstrap_test_prepare)

ds_build_assets_dir(assetsDir deepsea_render_bootstrap_test)
set(assetsDir ${assetsDir}/RenderBootstrapTest-assets)
add_custom_target(deepsea_render_bootstrap_assets
	DEPENDS deepsea_render_bootstrap_test_shaders
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${shaderDir} ${assetsDir}
	COMMENT "Copying assets for TestRenderBootstrap")
add_dependencies(deepsea_render_bootstrap_test deepsea_render_bootstrap_assets)

ds_set_folder(deepsea_render_bootstrap_test tests/unit)
ds_set_folder(deepsea_render_bootstrap_test_prepare tests/unit/Resources)
ds_set_folder(deepsea_render_bootstrap_test_shaders tests/unit/Resources)
ds_set_folder(deepsea_render_bootstrap_assets tests/unit/Resources)

add_test(NAME DeepSeaRenderBootstrapTest COMMAND deepsea_render_bootstrap_test)
