if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(deepsea_geometry_test ${sources})

target_include_directories(deepsea_geometry_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(deepsea_geometry_test PRIVATE
	deepsea_geometry
	${GTEST_BOTH_LIBRARIES})

ds_set_folder(deepsea_geometry_test tests)
add_test(NAME DeepSeaGeometryTest COMMAND deepsea_geometry_test)
