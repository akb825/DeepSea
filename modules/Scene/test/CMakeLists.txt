if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS OR NOT TARGET deepsea_render_mock)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_scene_test ${sources})

target_include_directories(deepsea_scene_test PRIVATE .)
target_link_libraries(deepsea_scene_test PRIVATE deepsea_scene deepsea_render_mock)

ds_set_folder(deepsea_scene_test tests/unit)
add_test(NAME DeepSeaSceneTest COMMAND deepsea_scene_test)
