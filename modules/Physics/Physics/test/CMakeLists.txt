if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_physics_test ${sources})

target_link_libraries(deepsea_physics_test PRIVATE DeepSea::Physics)

ds_set_folder(deepsea_physics_test tests/unit)
add_test(NAME DeepSeaPhysicsTest COMMAND deepsea_physics_test)
