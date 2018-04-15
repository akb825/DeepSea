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
	VectorElementShape.msl VectorElementImage.msl
	VectorElementShapeSRGB.msl)
if (TARGET deepsea_render_opengl)
	list(APPEND source SetupOpenGL.c SetupOpenGL.h)
	list(APPEND libraries deepsea_render_opengl)
	list(APPEND defines DS_HAS_OPENGL=1)
	list(APPEND shaderConfigs glsl-1.1 glsl-1.5 glsl-4.0 glsl-es-1.0 glsl-es-3.0 glsl-es-3.2)
	list(APPEND srgbShaderConfigs glsl-1.5 glsl-4.0 glsl-es-3.0 glsl-es-3.2)
else()
	list(APPEND defines DS_HAS_OPENGL=0)
endif()

if (NOT libraries)
	return()
endif()

add_executable(deepsea_test_vector_draw_app WIN32 ${source})
target_link_libraries(deepsea_test_vector_draw_app PRIVATE
	deepsea_application_sdl deepsea_application_sdl_main deepsea_vector_draw
	${libraries})
target_compile_definitions(deepsea_test_vector_draw_app PRIVATE ${defines})

set(imageDir ${CMAKE_CURRENT_BINARY_DIR}/images)
set(shaderDir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
add_custom_target(deepsea_test_vector_draw_app_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${imageDir})

# Shaders
set(shaderOptions OUTPUT_DIR ${shaderDir}
	INCLUDE ${DEEPSEA_MODULE_DIR}/Render/include ${DEEPSEA_MODULE_DIR}/Text/include
		${DEEPSEA_MODULE_DIR}/VectorDraw/include
	DEPENDENCY_RECURSE ${DEEPSEA_MODULE_DIR}/Render/include/*.mslh
		${DEEPSEA_MODULE_DIR}/Text/include/*.mslh ${DEEPSEA_MODULE_DIR}/VectorDraw/include/*.mslh)
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/VectorElementImage.msl
	${CMAKE_CURRENT_SOURCE_DIR}/VectorElementShape.msl OUTPUT TestVectorDraw.mslb
	CONFIG ${shaderConfigs} ${shaderOptions})
ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/VectorElementShapeSRGB.msl
	OUTPUT TestVectorDrawSRGB.mslb CONFIG ${srgbShaderConfigs} ${shaderOptions})
ds_compile_shaders_target(deepsea_test_vector_draw_app_shaders shaders
	DEPENDS deepsea_test_vector_draw_app_prepare)
add_dependencies(deepsea_test_vector_draw_app deepsea_test_vector_draw_app_shaders)

# SVGs
file(GLOB svgFiles SVGs/*.svg)
foreach (svg ${svgFiles})
	get_filename_component(svgName ${svg} NAME_WE)
	ds_convert_svg(svgs FILE ${svg} OUTPUT ${imageDir}/${svgName}.dsvi)
endforeach()
ds_convert_svg_target(deepsea_test_vector_draw_app_svgs svgs
	DEPENDS deepsea_test_vector_draw_app_prepare)
add_dependencies(deepsea_test_vector_draw_app deepsea_test_vector_draw_app_svgs)

set(fontDir ${CMAKE_CURRENT_SOURCE_DIR}/../Shared/Fonts)

set(assetsDir $<TARGET_FILE_DIR:deepsea_test_vector_draw_app>/TestVectorDraw-assets)
add_custom_target(deepsea_test_vector_draw_app_assets ALL DEPENDS deepsea_test_vector_draw_app
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${shaderDir} ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${imageDir} ${assetsDir}
	COMMENT "Copying assets for TestVectorDraw")

ds_set_folder(deepsea_test_vector_draw_app tests)
ds_set_folder(deepsea_test_vector_draw_app_prepare tests/Resources)
ds_set_folder(deepsea_test_vector_draw_app_shaders tests/Resources)
ds_set_folder(deepsea_test_vector_draw_app_assets tests/Resources)
