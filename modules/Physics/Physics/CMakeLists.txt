file(GLOB_RECURSE sources src/*.c src/*.cpp src/*.h include/*.h include/*.mslh)
file(GLOB_RECURSE publicFlatbuffers include/*.fbs)
file(GLOB_RECURSE privateFlatbuffers src/*.fbs)
ds_add_library(deepsea_physics MODULE Physics FILES ${sources} ${publicFlatbuffers}
	${privateFlatbuffers})
ds_target_include_directories(deepsea_physics PRIVATE src ${FLATBUFFERS_INCLUDE_DIRS})
ds_install_library(TARGET deepsea_physics MODULE Physics DEPENDS Geometry)

ds_convert_flatbuffers(generatedFlatbuffers FILE ${publicFlatbuffers}
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/DeepSea/Physics/Flatbuffers
	INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include
	PYTHON ${DEEPSEA_SOURCE_DIR}/python)
ds_convert_flatbuffers(generatedFlatbuffers FILE ${privateFlatbuffers}
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/Flatbuffers
	INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include
	PYTHON ${DEEPSEA_SOURCE_DIR}/python)
ds_convert_flatbuffers_target(deepsea_physics_flatbuffers generatedFlatbuffers)
add_dependencies(deepsea_physics deepsea_physics_flatbuffers)
ds_set_folder(deepsea_physics_flatbuffers modules/Flatbuffers)

add_subdirectory(test)
