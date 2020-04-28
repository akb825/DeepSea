if (NOT TARGET deepsea_application_sdl OR NOT TARGET deepsea_vector_draw)
	return()
endif()

if (NOT MSLC)
	message("mslc shader compiler not found, skipping TestVectorDraw.")
	return()
endif()

if (NOT CUTTLEFISH)
	message("cuttlefish texture converter not found, skipping TestVectorDraw.")
	return()
endif()

set(shaderConfigs)
set(srgbShaderConfigs)
set(source TestVectorDraw.c
	VectorElementFill.msl VectorElementLine.msl VectorElementImage.msl VectorElementTextColor.msl
		 VectorElementTextGradient.msl
	VectorElementFillSRGB.msl VectorElementLineSRGB.msl VectorElementTextColorSRGB.msl
		VectorElementTextGradientSRGB.msl)
if (TARGET deepsea_render_opengl)
	list(APPEND shaderConfigs glsl-1.1 glsl-1.5 glsl-4.0 glsl-es-1.0 glsl-es-3.0 glsl-es-3.2)
	list(APPEND srgbShaderConfigs glsl-1.5 glsl-4.0 glsl-es-3.0 glsl-es-3.2)
endif()
if (TARGET deepsea_render_vulkan)
	list(APPEND shaderConfigs spirv-1.0)
	list(APPEND srgbShaderConfigs spirv-1.0)
endif()
if (TARGET deepsea_render_metal)
	if (IOS)
		list(APPEND shaderConfigs metal-ios-1.1)
		list(APPEND srgbShaderConfigs metal-ios-1.1)
	else()
		list(APPEND shaderConfigs metal-osx-1.1)
		list(APPEND srgbShaderConfigs metal-osx-1.1)
	endif()
endif()

if (NOT shaderConfigs)
	return()
endif()

ds_add_executable(deepsea_test_vector_draw_app WIN32 ${source})
target_link_libraries(deepsea_test_vector_draw_app PRIVATE deepsea_render_bootstrap
	deepsea_application_sdl deepsea_vector_draw)
ds_link_main_lib(deepsea_test_vector_draw_app deepsea_application_sdl_main)

if (TARGET deepsea_easy_profiler)
	target_link_libraries(deepsea_test_vector_draw_app PRIVATE deepsea_easy_profiler)
	target_compile_definitions(deepsea_test_vector_draw_app PRIVATE DS_HAS_EASY_PROFILER=1)
endif()

ds_config_binary_dir(shaderDir shaders)
set(resourceDir ${CMAKE_CURRENT_BINARY_DIR}/resources)
set(imageDir ${CMAKE_CURRENT_BINARY_DIR}/images)
add_custom_target(deepsea_test_vector_draw_app_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${resourceDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${imageDir})

# Shaders
# Strip symbols and optimize to verify that this works with non-trivial shaders.
set(shaderOptions OUTPUT_DIR ${shaderDir}
	INCLUDE ${DEEPSEA_MODULE_DIR}/Render/include ${DEEPSEA_MODULE_DIR}/Text/include
		${DEEPSEA_MODULE_DIR}/VectorDraw/include
	DEPENDS_RECURSE ${DEEPSEA_MODULE_DIR}/Render/include/*.mslh
		${DEEPSEA_MODULE_DIR}/Text/include/*.mslh ${DEEPSEA_MODULE_DIR}/VectorDraw/include/*.mslh)
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/VectorElementFill.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementLine.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementImage.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementTextColor.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementTextGradient.msl
	OUTPUT TestVectorDraw.mslb
	CONFIG ${shaderConfigs} ${shaderOptions})
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/VectorElementFillSRGB.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementLineSRGB.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementImage.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementTextColorSRGB.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementTextGradientSRGB.msl
	OUTPUT TestVectorDrawSRGB.mslb
	CONFIG ${srgbShaderConfigs} ${shaderOptions})
ds_compile_shaders_target(deepsea_test_vector_draw_app_shaders shaders
	DEPENDS deepsea_test_vector_draw_app_prepare)

# Shared resources
ds_create_vector_resources(resources FILE ${CMAKE_CURRENT_SOURCE_DIR}/resources.json
	OUTPUT ${resourceDir}/resources.dsvr)
ds_create_vector_resources_target(deepsea_test_vector_draw_app_resources resources
	DEPENDS deepsea_test_vector_draw_app_prepare)

# SVGs
file(GLOB svgFiles SVGs/*.svg)
foreach (svg ${svgFiles})
	get_filename_component(svgName ${svg} NAME_WE)
	ds_convert_svg(svgs FILE ${svg} OUTPUT ${imageDir}/${svgName}.dsvi)
endforeach()
ds_convert_svg_target(deepsea_test_vector_draw_app_svgs svgs
	DEPENDS deepsea_test_vector_draw_app_prepare)

ds_build_assets_dir(assetsDir deepsea_test_vector_draw_app)
set(assetsDir ${assetsDir}/TestVectorDraw-assets)
add_custom_target(deepsea_test_vector_draw_app_assets
	DEPENDS deepsea_test_vector_draw_app_shaders deepsea_test_vector_draw_app_resources
		deepsea_test_vector_draw_app_svgs
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${shaderDir} ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${resourceDir} ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${imageDir} ${assetsDir}
	COMMENT "Copying assets for TestVectorDraw")
add_dependencies(deepsea_test_vector_draw_app deepsea_test_vector_draw_app_assets)

ds_set_folder(deepsea_test_vector_draw_app tests/gui)
ds_set_folder(deepsea_test_vector_draw_app_prepare tests/gui/Resources)
ds_set_folder(deepsea_test_vector_draw_app_shaders tests/gui/Resources)
ds_set_folder(deepsea_test_vector_draw_app_resources tests/gui/Resources)
ds_set_folder(deepsea_test_vector_draw_app_svgs tests/gui/Resources)
ds_set_folder(deepsea_test_vector_draw_app_assets tests/gui/Resources)
