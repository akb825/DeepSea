if (NOT TARGET deepsea_application_sdl OR NOT TARGET deepsea_text)
	return()
endif()

if (NOT MSLC)
	message("mslc shader compiler not found, skipping TestRenderSubpass.")
	return()
endif()

set(libraries)
set(defines)
set(shaderConfigs)
set(source TestText.c Font.msl FontTess.msl)
if (TARGET deepsea_render_opengl)
	list(APPEND source SetupOpenGL.c SetupOpenGL.h)
	list(APPEND libraries deepsea_render_opengl)
	list(APPEND defines DS_HAS_OPENGL=1)
	list(APPEND shaderConfigs glsl-1.1 glsl-1.5 glsl-4.0 glsl-es-1.0 glsl-es-3.0 glsl-es-3.2)
else()
	list(APPEND defines DS_HAS_OPENGL=0)
endif()

if (NOT libraries)
	return()
endif()

add_executable(deepsea_test_text_app WIN32 ${source})
target_link_libraries(deepsea_test_text_app PRIVATE
	deepsea_application_sdl deepsea_application_sdl_main deepsea_text
	${libraries})
target_compile_definitions(deepsea_test_text_app PRIVATE ${defines})

# Cannot use generator expressions for the custom commands.
set(shaderDir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
add_custom_target(deepsea_test_text_app_prepare
	COMMAND ${CMAKE_COMMAND} -E make_directory ${shaderDir})

ds_compile_shaders(shaders FILE ${CMAKE_CURRENT_SOURCE_DIR}/Font.msl
	${CMAKE_CURRENT_SOURCE_DIR}/FontTess.msl
	OUTPUT TestText.mslb CONFIG ${shaderConfigs} OUTPUT_DIR ${shaderDir}
	INCLUDE ${DEEPSEA_SOURCE_DIR}/Render/include ${DEEPSEA_SOURCE_DIR}/Text/include
	DEPENDENCY_RECURSE ${DEEPSEA_SOURCE_DIR}/Render/include/*.mslh
		${DEEPSEA_SOURCE_DIR}/Text/include/*.mslh)
ds_compile_shaders_target(deepsea_test_text_app_shaders shaders
	DEPENDS deepsea_test_text_app_prepare)
add_dependencies(deepsea_test_text_app deepsea_test_text_app_shaders)

set(fontDir ${CMAKE_CURRENT_SOURCE_DIR}/../Shared/Fonts)

set(assetsDir $<TARGET_FILE_DIR:deepsea_test_text_app>/TestText-assets)
add_custom_target(deepsea_test_text_app_assets ALL DEPENDS deepsea_test_text_app
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${shaderDir} ${assetsDir}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${fontDir} ${assetsDir}/Fonts
	COMMENT "Copying assets for TestRenderSubpass")

ds_set_folder(deepsea_test_text_app tests)
ds_set_folder(deepsea_test_text_app_prepare tests/Resources)
ds_set_folder(deepsea_test_text_app_shaders tests/Resources)
ds_set_folder(deepsea_test_text_app_assets tests/Resources)
