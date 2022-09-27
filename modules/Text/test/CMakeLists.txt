if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_text_test ${sources})

target_link_libraries(deepsea_text_test PRIVATE DeepSea::Text)

ds_set_folder(deepsea_text_test tests/unit)
add_test(NAME DeepSeaTextTest COMMAND deepsea_text_test)
