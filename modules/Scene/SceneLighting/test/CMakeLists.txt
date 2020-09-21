if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_scene_lighting_test ${sources})

target_include_directories(deepsea_scene_lighting_test PRIVATE .)
target_link_libraries(deepsea_scene_lighting_test PRIVATE deepsea_scene_lighting)

ds_set_folder(deepsea_scene_lighting_test tests/unit)
add_test(NAME DeepSeaSceneLightingTest COMMAND deepsea_scene_lighting_test)
