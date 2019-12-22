file(GLOB_RECURSE sources src/*.c src/*.cpp src/*.h include/*.h include/*.mslh)
file(GLOB_RECURSE flatbuffers src/*.fbs)
ds_add_library(deepsea_scene MODULE Scene FILES ${sources} ${flatbuffers} DEPENDS deepsea_render)
ds_target_include_directories(deepsea_scene PRIVATE ${DEEPSEA_MODULE_DIR}/Scene/src)
ds_install_library(TARGET deepsea_scene MODULE Scene DEPENDS Render)

include(ConvertFlatbuffers)
ds_convert_flatbuffers(generatedFlatbuffers FILE ${flatbuffers} DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}/src/Flatbuffers PYTHON ${DEEPSEA_SOURCE_DIR}/python)
ds_convert_flatbuffers_target(deepsea_scene_flatbuffers generatedFlatbuffers)
add_dependencies(deepsea_scene deepsea_scene_flatbuffers)
ds_set_folder(deepsea_scene_flatbuffers modules/Flatbuffers)

add_subdirectory(test)
