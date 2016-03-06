if (MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /warn:3 /warnaserror")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /warn:3 /warnaserror")
else()
	if (DEEPSEA_SHARED AND
		(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang"))
		set(otherCFlags "-fvisibility=hidden")
		set(otherCXXFlags "${otherCFlags} -fvisibility-inlines-hidden")
	else()
		set(otherCFlags)
		set(otherCXXFlags)
	endif()

	set(commonFlags "-Wall -Werror -fno-strict-aliasing")

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${commonFlags} ${otherCFlags}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${commonFlags} -std=c++11 ${otherCXXFlags}")
endif()

enable_testing()
