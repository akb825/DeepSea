if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS OR NOT TARGET deepsea_render_mock)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_vector_draw_test ${sources})

target_include_directories(deepsea_vector_draw_test PRIVATE ${DEEPSEA_MODULE_DIR}/VectorDraw/src)
target_link_libraries(deepsea_vector_draw_test PRIVATE
	deepsea_vector_draw
	deepsea_render_mock)

ds_set_folder(deepsea_vector_draw_test tests/unit)
add_test(NAME DeepSeaVectorDrawTest COMMAND deepsea_vector_draw_test)
