if (APPLE)
	message("OpenGL skipped for Apple platforms.")
	return()
endif()

set(extraLibs)
set(extraIncludeDirs)
set(extraDefines)
if (APPLE)
	set(extraLibs "-framework AppKit" "-framework OpenGL")
	set(extraDefines GL_SILENCE_DEPRECATION=1)
elseif (DEEPSEA_GLES OR ANDROID)
	find_package(EGL QUIET)
	if (NOT EGL_FOUND)
		message("OpenGL not found. Skipping RenderOpenGL.")
		return()
	endif()

	set(extraIncludeDirs ${EGL_INCLUDE_DIRS})
	set(extraDefines ANYGL_GLES=1)
elseif (UNIX)
	# Don't want to assume a windowing system, so everything is dynamically loaded. EGL requires the
	# headers to be available at compile-time, but GLX is purely dynamic.
	set(extraDefines ANYGL_HAS_GLX=1)

	find_package(EGL QUIET)
	if (EGL_FOUND)
		set(extraIncludeDirs ${EGL_INCLUDE_DIRS})
		list(APPEND extraDefines ANYGL_HAS_EGL=1)
	else()
		list(APPEND extraDefines ANYGL_HAS_EGL=0)
	endif()
else()
	find_package(OpenGL)
	if (NOT OPENGL_FOUND)
		message("OpenGL not found. Skipping RenderOpenGL.")
		return()
	endif()

	set(extraLibs ${OPENGL_gl_LIBRARY})
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/msl-config/ConfigPaths.cmake)

set(extraPatterns)
if (APPLE)
	set(extraPatterns src/*.m)
endif()
file(GLOB_RECURSE sources src/*.c src/*.h include/*.h ${extraPatterns})
ds_add_library(deepsea_render_opengl MODULE RenderOpenGL FILES ${sources})

ds_target_include_directories(deepsea_render_opengl
	PRIVATE ${OPENGL_INCLUDE_DIR} ${extraIncludeDirs} src)
ds_target_link_libraries(deepsea_render_opengl
	PRIVATE ${CMAKE_DL_LIBS} ${extraLibs} MSL::Client)
if (extraDefines)
	ds_target_compile_definitions(deepsea_render_opengl PRIVATE ${extraDefines})
endif()

file(GLOB shaderConfigs msl-config/*)
ds_install_library(TARGET deepsea_render_opengl MODULE RenderOpenGL DEPENDS Render
	CONFIG_FILES ${shaderConfigs}
	CONFIG_LINES "include(\${CMAKE_CURRENT_LIST_DIR}/ConfigPaths.cmake)")
