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

find_package(PythonInterp QUIET)
if (NOT PYTHONINTERP_FOUND)
	message("Python not found, skipping TestVectorDraw.")
	return()
endif()

set(libraries)
set(defines)
set(shaderConfigs)
set(srgbShaderConfigs)
set(source TestVectorDraw.c
	VectorElementFill.msl VectorElementLine.msl VectorElementImage.msl VectorElementText.msl
	VectorElementFillSRGB.msl VectorElementLineSRGB.msl VectorElementTextSRGB.msl)
if (TARGET deepsea_render_opengl)
	list(APPEND shaderConfigs glsl-1.1 glsl-1.5 glsl-4.0 glsl-es-1.0 glsl-es-3.0 glsl-es-3.2)
	list(APPEND srgbShaderConfigs glsl-1.5 glsl-4.0 glsl-es-3.0 glsl-es-3.2)
endif()

if (NOT shaderConfigs)
	return()
endif()

if (TARGET deepsea_easy_profiler)
	list(APPEND libraries deepsea_easy_profiler)
	list(APPEND defines DS_HAS_EASY_PROFILER=1)
endif()

ds_add_executable(deepsea_test_vector_draw_app WIN32 ${source})
target_link_libraries(deepsea_test_vector_draw_app PRIVATE deepsea_render_bootstrap
	deepsea_application_sdl deepsea_vector_draw ${libraries})
ds_link_main_lib(deepsea_test_vector_draw_app deepsea_application_sdl_main)
target_compile_definitions(deepsea_test_vector_draw_app PRIVATE ${defines})

set(shaderDir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
set(resourceDir ${CMAKE_CURRENT_BINARY_DIR}/resources)
set(imageDir ${CMAKE_CURRENT_BINARY_DIR}/images)
add_custom_target(deepsea_test_vector_draw_app_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${resourceDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${imageDir})

# Shaders
set(shaderOptions OUTPUT_DIR ${shaderDir}
	INCLUDE ${DEEPSEA_MODULE_DIR}/Render/include ${DEEPSEA_MODULE_DIR}/Text/include
		${DEEPSEA_MODULE_DIR}/VectorDraw/include
	DEPENDENCY_RECURSE ${DEEPSEA_MODULE_DIR}/Render/include/*.mslh
		${DEEPSEA_MODULE_DIR}/Text/include/*.mslh ${DEEPSEA_MODULE_DIR}/VectorDraw/include/*.mslh)
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/VectorElementFill.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementLine.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementImage.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementText.msl OUTPUT TestVectorDraw.mslb
	CONFIG ${shaderConfigs} ${shaderOptions})
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/VectorElementFillSRGB.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementLineSRGB.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementImage.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementTextSRGB.msl  OUTPUT TestVectorDrawSRGB.mslb
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

ds_build_assets_dir(assetsDir)
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
