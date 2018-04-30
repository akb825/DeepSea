if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(deepsea_text_test ${sources})

target_include_directories(deepsea_text_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(deepsea_text_test PRIVATE
	deepsea_text
	${GTEST_BOTH_LIBRARIES})

ds_set_folder(deepsea_text_test tests/unit)
add_test(NAME DeepSeaTextTest COMMAND deepsea_text_test)
