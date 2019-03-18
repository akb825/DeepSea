if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

set(shaderConfigs)
set(computeShaderConfigs)
if (TARGET deepsea_render_opengl)
	list(APPEND shaderConfigs glsl-1.1 glsl-es-1.0)
	list(APPEND computeShaderConfigs glsl-4.1 glsl-es-3.1)
endif()
if (TARGET deepsea_render_vulkan)
	list(APPEND computeShaderConfigs spirv-1.0)
endif()

if (NOT shaderConfigs AND NOT computeShaderConfigs)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h *.msl *.mslh)
ds_add_unittest(deepsea_render_bootstrap_test ${sources})

target_include_directories(deepsea_render_bootstrap_test PRIVATE .)
target_link_libraries(deepsea_render_bootstrap_test PRIVATE deepsea_render_bootstrap)

# Cannot use generator expressions for the custom commands.
set(shaderDir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
add_custom_target(deepsea_render_bootstrap_test_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir})

set(shaderSourceDir ${CMAKE_CURRENT_SOURCE_DIR}/Shaders)
ds_compile_shaders(shaders FILE ${shaderSourceDir}/WriteBuffer.msl
	OUTPUT Writeuffer.mslb CONFIG ${computeShaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${DEEPSEA_MODULE_DIR}/Render/include)
ds_compile_shaders(shaders FILE ${shaderSourceDir}/WriteOffscreen.msl
	OUTPUT WriteOffscreen.mslb CONFIG ${shaderConfigs} ${computeShaderConfigs}
	OUTPUT_DIR ${shaderDir} INCLUDE ${DEEPSEA_MODULE_DIR}/Render/include)
ds_compile_shaders_target(deepsea_render_bootstrap_test_shaders shaders
	DEPENDS deepsea_render_bootstrap_test_prepare)

ds_build_assets_dir(assetsDir)
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