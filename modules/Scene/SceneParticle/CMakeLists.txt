file(GLOB_RECURSE sources src/*.c src/*.cpp src/*.h include/*.h include/*.mslh)
file(GLOB_RECURSE publicFlatbuffers include/*.fbs)
file(GLOB_RECURSE privateFlatbuffers src/*.fbs)
ds_add_library(deepsea_scene_particle MODULE SceneParticle FILES ${sources}
	${publicFlatbuffers} ${privateFlatbuffers} DEPENDS deepsea_scene deepsea_particle)
ds_target_include_directories(deepsea_scene_particle PRIVATE src ${FLATBUFFERS_INCLUDE_DIRS})
ds_install_library(TARGET deepsea_scene_particle MODULE SceneParticle DEPENDS Scene Particle)

ds_convert_flatbuffers(generatedFlatbuffers FILE ${publicFlatbuffers}
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/DeepSea/SceneParticle/Flatbuffers
	INCLUDE ${DEEPSEA_MODULE_DIR}/Scene/Scene/include ${CMAKE_CURRENT_SOURCE_DIR}/include
	PYTHON ${DEEPSEA_SOURCE_DIR}/python)
ds_convert_flatbuffers(generatedFlatbuffers FILE ${privateFlatbuffers}
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/Flatbuffers
	INCLUDE ${DEEPSEA_MODULE_DIR}/Scene/Scene/include ${CMAKE_CURRENT_SOURCE_DIR}/include
	PYTHON ${DEEPSEA_SOURCE_DIR}/python)
ds_convert_flatbuffers_target(deepsea_scene_particle_flatbuffers generatedFlatbuffers)
add_dependencies(deepsea_scene_particle deepsea_scene_particle_flatbuffers)
ds_set_folder(deepsea_scene_particle_flatbuffers modules/Flatbuffers)
