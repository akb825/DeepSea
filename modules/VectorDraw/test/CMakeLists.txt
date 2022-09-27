if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS OR NOT TARGET DeepSea::RenderMock)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_vector_draw_test ${sources})

target_include_directories(deepsea_vector_draw_test PRIVATE ${DEEPSEA_MODULE_DIR}/VectorDraw/src)
target_link_libraries(deepsea_vector_draw_test
	PRIVATE	DeepSea::VectorDraw DeepSea::RenderMock)

ds_set_folder(deepsea_vector_draw_test tests/unit)
add_test(NAME DeepSeaVectorDrawTest COMMAND deepsea_vector_draw_test)
