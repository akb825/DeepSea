find_library(MATH_LIB m)
if (NOT MATH_LIB)
	set(MATH_LIB "")
endif()

ds_glob_library_sources(sources Math src/*.c src/*.h include/*.h)
ds_add_library(deepsea_math Math ${sources})

ds_target_link_libraries(deepsea_math PUBLIC ${MATH_LIB})
ds_target_compile_definitions(deepsea_math PUBLIC _USE_MATH_DEFINES)
