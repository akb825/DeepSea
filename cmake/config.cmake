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

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror ${otherCFlags}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11 ${otherCXXFlags}")
endif()

enable_testing()
