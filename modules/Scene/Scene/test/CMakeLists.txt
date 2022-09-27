if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS OR NOT TARGET DeepSea::RenderMock)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_scene_test ${sources})

target_include_directories(deepsea_scene_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(deepsea_scene_test PRIVATE DeepSea::Scene DeepSea::RenderMock)

ds_set_folder(deepsea_scene_test tests/unit)
add_test(NAME DeepSeaSceneTest COMMAND deepsea_scene_test)
