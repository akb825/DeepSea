if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(deepsea_render_test ${sources})

target_include_directories(deepsea_render_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(deepsea_render_test PRIVATE
	deepsea_render
	${GTEST_BOTH_LIBRARIES})

ds_set_folder(deepsea_render_test tests)
add_test(NAME DeepSeaRenderTest COMMAND deepsea_render_test)
