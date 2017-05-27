set(extraLibs)
set(extraIncludeDirs)
if (${DEEPSEA_GLES} AND NOT ${APPLE})
	find_package(EGL REQUIRED)
	set(extraLibs ${EGL_LIBRARIES})
	set(extraIncludeDirs ${EGL_INCLUDE_DIRS})
elseif (UNIX AND NOT APPLE)
	find_package(X11 REQUIRED)
	find_package(GLX REQUIRED)
	set(extraLibs ${X11_LIBRARIES} ${GLX_LIBRARIES})
	set(extraIncludeDirs ${X11_INCLUDE_DIRECTORIES} ${GLX_INCLUDE_DIRECTORIES})
endif()

ds_glob_library_sources(sources RenderOpenGL src/*.c src/*.h include/*.h)
ds_add_library(deepsea_render_opengl RenderOpenGL ${sources})
ds_target_include_directories(deepsea_render_opengl
	PRIVATE ${OPENGL_INCLUDE_DIR} ${extraIncludeDirs} ${DEEPSEA_SOURCE_DIR}/RenderOpenGL/src)
ds_target_link_libraries(deepsea_render_opengl
	PRIVATE ${CMAKE_DL_LIBS} ${OPENGL_gl_LIBRARIES} ${extraLibs})
