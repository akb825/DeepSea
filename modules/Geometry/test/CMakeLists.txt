if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_geometry_test ${sources})

target_link_libraries(deepsea_geometry_test PRIVATE deepsea_geometry)

ds_set_folder(deepsea_geometry_test tests/unit)
add_test(NAME DeepSeaGeometryTest COMMAND deepsea_geometry_test)
