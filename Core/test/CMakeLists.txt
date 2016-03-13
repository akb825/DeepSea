if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(deepsea_core_test ${sources})

target_include_directories(deepsea_core_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(deepsea_core_test PRIVATE
	deepsea_core
	${GTEST_BOTH_LIBRARIES})

set_folder(deepsea_core_test tests)
add_test(NAME DeepSeaCoreTest COMMAND deepsea_core_test)
