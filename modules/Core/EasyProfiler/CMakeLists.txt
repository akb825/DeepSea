if (IOS)
	message("EasyProfiler currently disabled on iOS due to invalid assembly instruction.")
	return()
endif()

file(GLOB_RECURSE sources src/*.c src/*.cpp src/*.h include/*.h)

if (DEEPSEA_PROFILING)
	set(easyProfilerCoreDir easy_profiler/easy_profiler_core)
	set(easyProfilerMajorVersion 2)
	set(easyProfilerMinorVersion 1)
	set(easyProfilerPatchVersion 0)
	file(GLOB_RECURSE externalSources ${easyProfilerCoreDir}/*.cpp ${easyProfilerCoreDir}/*.h)
else()
	set(externalSources)
endif()

ds_add_library(deepsea_easy_profiler MODULE EasyProfiler FILES ${sources}
	EXTERNAL_FILES ${externalSources})

ds_target_include_directories(deepsea_easy_profiler PRIVATE ${easyProfilerCoreDir}/include)
ds_target_compile_definitions(deepsea_easy_profiler PRIVATE
	EASY_PROFILER_VERSION_MAJOR=${easyProfilerMajorVersion}
	EASY_PROFILER_VERSION_MINOR=${easyProfilerMinorVersion}
	EASY_PROFILER_VERSION_PATCH=${easyProfilerPatchVersion}
	EASY_PROFILER_DEFAULT_PORT=28077 EASY_OPTION_EVENT_TRACING_ENABLED=0
	BUILD_WITH_EASY_PROFILER=1 EASY_PROFILER_STATIC=1 _BUILD_PROFILER=1)
ds_install_library(TARGET deepsea_easy_profiler MODULE EasyProfiler DEPENDS Core)
