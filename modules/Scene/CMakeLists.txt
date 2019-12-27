file(GLOB_RECURSE sources src/*.c src/*.cpp src/*.h include/*.h include/*.mslh)
file(GLOB_RECURSE publicFlatbuffers include/*.fbs)
file(GLOB_RECURSE privateFlatbuffers src/*.fbs)
ds_add_library(deepsea_scene MODULE Scene FILES ${sources} ${publicFlatbuffers}
	${privateFlatbuffers} DEPENDS deepsea_render)
ds_target_include_directories(deepsea_scene PRIVATE ${DEEPSEA_MODULE_DIR}/Scene/src
	${FLATBUFFERS_INCLUDE_DIRS})
ds_install_library(TARGET deepsea_scene MODULE Scene DEPENDS Render)

include(ConvertFlatbuffers)
ds_convert_flatbuffers(generatedFlatbuffers FILE ${publicFlatbuffers}
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/DeepSea/Scene/Flatbuffers
	INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include
	PYTHON ${DEEPSEA_SOURCE_DIR}/python)
ds_convert_flatbuffers(generatedFlatbuffers FILE ${privateFlatbuffers}
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/Flatbuffers
	INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include
	PYTHON ${DEEPSEA_SOURCE_DIR}/python)
ds_convert_flatbuffers_target(deepsea_scene_flatbuffers generatedFlatbuffers)
add_dependencies(deepsea_scene deepsea_scene_flatbuffers)
ds_set_folder(deepsea_scene_flatbuffers modules/Flatbuffers)

add_subdirectory(test)
