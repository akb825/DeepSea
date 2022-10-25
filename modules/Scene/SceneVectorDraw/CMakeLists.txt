file(GLOB_RECURSE sources src/*.c src/*.cpp src/*.h include/*.h)
file(GLOB_RECURSE flatbuffers src/*.fbs)
ds_add_library(deepsea_scene_vector_draw MODULE SceneVectorDraw FILES ${sources} ${flatbuffers})
ds_target_include_directories(deepsea_scene_vector_draw PRIVATE src ${FLATBUFFERS_INCLUDE_DIRS})
ds_install_library(TARGET deepsea_scene_vector_draw MODULE SceneVectorDraw DEPENDS VectorDraw Scene)

ds_convert_flatbuffers(generatedFlatbuffers FILE ${flatbuffers}
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/Flatbuffers
	INCLUDE ${DEEPSEA_MODULE_DIR}/Scene/Scene/include
	PYTHON ${DEEPSEA_SOURCE_DIR}/python)
ds_convert_flatbuffers_target(deepsea_scene_vector_draw_flatbuffers generatedFlatbuffers)
add_dependencies(deepsea_scene_vector_draw deepsea_scene_vector_draw_flatbuffers)
ds_set_folder(deepsea_scene_vector_draw_flatbuffers modules/Flatbuffers)
