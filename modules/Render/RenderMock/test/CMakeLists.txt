if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_render_mock_test ${sources})

target_include_directories(deepsea_render_mock_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(deepsea_render_mock_test PRIVATE DeepSea::RenderMock)
ds_build_assets_dir(assetsDir deepsea_render_mock_test)
add_custom_command(TARGET deepsea_render_mock_test POST_BUILD
	COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets/
	${assetsDir}/RenderMock-assets)

ds_set_folder(deepsea_render_mock_test tests/unit)
add_test(NAME DeepSeaRenderMockTest COMMAND deepsea_render_mock_test)
