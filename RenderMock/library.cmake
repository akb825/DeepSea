ds_glob_library_sources(sources RenderMock src/*.c src/*.h include/*.h)
ds_add_library(deepsea_render_mock RenderMock ${sources})