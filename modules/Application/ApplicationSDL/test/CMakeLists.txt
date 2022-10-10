if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_application_sdl_test ${sources})

target_include_directories(deepsea_application_sdl_test PRIVATE
	../src
	${SDL2_INCLUDE_DIRS})
target_link_libraries(deepsea_application_sdl_test PRIVATE DeepSea::ApplicationSDL)

ds_set_folder(deepsea_application_sdl_test tests/unit)
add_test(NAME DeepSeaApplicationSdlTest COMMAND deepsea_application_sdl_test)
