file(GLOB_RECURSE sources src/*.c src/*.cpp src/*.h include/*.h include/*.mslh)
file(GLOB_RECURSE flatbuffers src/*.fbs)
ds_add_library(deepsea_scene_lighting MODULE SceneLighting FILES ${sources}
	${flatbuffers} DEPENDS deepsea_scene)
ds_target_include_directories(deepsea_scene_lighting PRIVATE src ${FLATBUFFERS_INCLUDE_DIRS})
ds_install_library(TARGET deepsea_scene_lighting MODULE SceneLighting DEPENDS Scene)

#ds_convert_flatbuffers(generatedFlatbuffers FILE ${flatbuffers}
#	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/Flatbuffers
#	INCLUDE ${DEEPSEA_MODULE_DIR}/Scene/Scene/include
#	PYTHON ${DEEPSEA_SOURCE_DIR}/python)
#ds_convert_flatbuffers_target(deepsea_vector_draw_scene_flatbuffers generatedFlatbuffers)
#add_dependencies(deepsea_vector_draw_scene deepsea_vector_draw_scene_flatbuffers)
#ds_set_folder(deepsea_vector_draw_scene_flatbuffers modules/Flatbuffers)

add_subdirectory(test)
