find_library(MATH_LIB m)
if (NOT MATH_LIB)
	set(MATH_LIB "")
endif()

file(GLOB_RECURSE sources src/*.c src/*.cpp src/*.h include/*.h)
ds_add_library(deepsea_math MODULE Math FILES ${sources})

ds_target_link_libraries(deepsea_math PUBLIC ${MATH_LIB})
ds_target_compile_definitions(deepsea_math PUBLIC _USE_MATH_DEFINES)

ds_install_library(TARGET deepsea_math MODULE Math DEPENDS Core)

if (DEEPSEA_ARCH MATCHES "^x86")
	ds_target_compile_definitions(deepsea_math PUBLIC DS_X86_ARCH_LEVEL=${DEEPSEA_X86_ARCH_LEVEL})
else()
	ds_target_compile_definitions(deepsea_math PUBLIC DS_X86_ARCH_LEVEL=0)
endif()

# Always build with Neon support.
if (DEEPSEA_ARCH STREQUAL "arm")
	ds_target_compile_options(deepsea_math PUBLIC -mfpu=neon-vfpv4)
endif()

add_subdirectory(test)
