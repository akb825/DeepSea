if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_animation_test ${sources})

target_include_directories(deepsea_animation_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(deepsea_animation_test PRIVATE DeepSea::Animation)

ds_set_folder(deepsea_animation_test tests/unit)
add_test(NAME DeepSeaAnimationTest COMMAND deepsea_animation_test)
