include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CreateSceneResources.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CreateScene.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CreateView.cmake)

file(GLOB_RECURSE sources src/*.c src/*.cpp src/*.h include/*.h include/*.mslh)
file(GLOB_RECURSE publicFlatbuffers include/*.fbs)
file(GLOB_RECURSE privateFlatbuffers src/*.fbs)
ds_add_library(deepsea_scene MODULE Scene FILES ${sources} ${publicFlatbuffers}
	${privateFlatbuffers})
ds_target_include_directories(deepsea_scene PRIVATE src ${FLATBUFFERS_INCLUDE_DIRS})
ds_install_library(TARGET deepsea_scene MODULE Scene DEPENDS Render
	CONFIG_FILES cmake/CreateSceneResources.cmake cmake/CreateScene.cmake cmake/CreateView.cmake
	CONFIG_LINES
		"include(\${CMAKE_CURRENT_LIST_DIR}/CreateSceneResources.cmake)"
		"include(\${CMAKE_CURRENT_LIST_DIR}/CreateScene.cmake)"
		"include(\${CMAKE_CURRENT_LIST_DIR}/CreateView.cmake)")

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
