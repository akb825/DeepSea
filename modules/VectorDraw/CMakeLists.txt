include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConvertSVG.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CreateVectorResources.cmake)

file(GLOB_RECURSE sources src/*.c src/*.cpp src/*.h include/*.h include/*.mslh)
file(GLOB_RECURSE flatbuffers src/*.fbs)
ds_add_library(deepsea_vector_draw MODULE VectorDraw FILES ${sources} ${flatbuffers}
	DEPENDS deepsea_render deepsea_text)
ds_target_include_directories(deepsea_vector_draw PRIVATE ${FLATBUFFERS_INCLUDE_DIRS})
ds_install_library(TARGET deepsea_vector_draw MODULE VectorDraw DEPENDS Render Text
	CONFIG_FILES cmake/ConvertSVG.cmake cmake/CreateVectorResources.cmake
	CONFIG_LINES
		"include(\${CMAKE_CURRENT_LIST_DIR}/ConvertSVG.cmake)"
		"include(\${CMAKE_CURRENT_LIST_DIR}/ConvertVectorResources.cmake)")

include(ConvertFlatbuffers)
ds_convert_flatbuffers(generatedFlatbuffers FILE ${flatbuffers} DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}/src/Flatbuffers PYTHON ${DEEPSEA_SOURCE_DIR}/python)
ds_convert_flatbuffers_target(deepsea_vector_draw_flatbuffers generatedFlatbuffers)
add_dependencies(deepsea_vector_draw deepsea_vector_draw_flatbuffers)
ds_set_folder(deepsea_vector_draw_flatbuffers modules/Flatbuffers)

add_subdirectory(test)
