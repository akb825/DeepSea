file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)
ds_add_library(deepsea_render_mock MODULE RenderMock FILES ${sources})

ds_target_include_directories(deepsea_render_mock PRIVATE src)
ds_target_link_libraries(deepsea_render_mock PRIVATE MSL::Client)

ds_install_library(TARGET deepsea_render_mock MODULE RenderMock DEPENDS Render)

add_subdirectory(test)
