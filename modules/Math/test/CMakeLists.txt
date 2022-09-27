if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_math_test ${sources})

target_link_libraries(deepsea_math_test PRIVATE DeepSea::Math)

ds_set_folder(deepsea_math_test tests/unit)
add_test(NAME DeepSeaMathTest COMMAND deepsea_math_test)
