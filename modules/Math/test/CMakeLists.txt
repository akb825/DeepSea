if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(deepsea_math_test ${sources})

target_include_directories(deepsea_math_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(deepsea_math_test PRIVATE
	deepsea_math
	${GTEST_BOTH_LIBRARIES})

ds_set_folder(deepsea_math_test tests/unit)
add_test(NAME DeepSeaMathTest COMMAND deepsea_math_test)
