file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)

set(dependencies)
set(implDefines)
if (TARGET deepsea_render_opengl)
	list(APPEND dependencies RenderOpenGL)
	list(APPEND implDefines DS_HAS_RENDER_OPENGL=1)
else()
	list(APPEND implDefines DS_HAS_RENDER_OPENGL=0)
endif()

if (TARGET deepsea_render_vulkan)
	list(APPEND dependencies RenderVulkan)
	list(APPEND implDefines DS_HAS_RENDER_VULKAN=1)
else()
	list(APPEND implDefines DS_HAS_RENDER_VULKAN=0)
endif()

if (TARGET deepsea_render_metal)
	list(APPEND dependencies RenderMetal)
	list(APPEND implDefines DS_HAS_RENDER_METAL=1)
else()
	list(APPEND implDefines DS_HAS_RENDER_METAL=0)
endif()

ds_add_library(deepsea_render_bootstrap MODULE RenderBootstrap FILES ${sources})
ds_target_compile_definitions(deepsea_render_bootstrap PUBLIC ${implDefines})

ds_install_library(TARGET deepsea_render_bootstrap MODULE RenderBootstrap
	DEPENDS Render ${dependencies})

add_subdirectory(test)
