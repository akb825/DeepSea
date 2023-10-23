if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_core_test ${sources})

target_link_libraries(deepsea_core_test PRIVATE DeepSea::Core)

ds_set_folder(deepsea_core_test tests/unit)
# Disable slow tests so they can be run as part of the build. Executing the test manually will
# also run the slower tests.
add_test(NAME DeepSeaCoreTest
	COMMAND deepsea_core_test --gtest_filter=-*TimedWait:*Sleep:*StressTest*)
