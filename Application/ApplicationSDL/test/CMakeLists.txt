if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(deepsea_application_sdl_test ${sources})

target_include_directories(deepsea_application_sdl_test PRIVATE
	../src
	${GTEST_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIR})
target_link_libraries(deepsea_application_sdl_test PRIVATE
	deepsea_application_sdl
	${GTEST_BOTH_LIBRARIES})

ds_set_folder(deepsea_application_sdl_test tests)
add_test(NAME DeepSeaApplicationSdlTest COMMAND deepsea_application_sdl_test)
