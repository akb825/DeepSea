if (APPLE AND NOT DEEPSEA_FORCE_OSS_TEXT)
	# TODO
	set(textDefines DS_APPLE_TEXT=1)
elseif(WIN32 AND NOT DEEPSEA_OSS_TEXT)
	# TODO
	set(textDefines DS_WIN32_TEXT=1)
else()
	find_package(Freetype REQUIRED)
	find_package(HarfBuzz REQUIRED)

	set(textIncludeDirs ${FREETYPE_INCLUDE_DIRS} ${HARFBUZZ_INCLUDE_DIRS})
	set(textLibraries ${FREETYPE_LIBRARIES} ${HARFBUZZ_LIBRARIES})
	set(textDefines DS_OSS_TEXT=1)
endif()

set(textIncludeDirs ${textIncludeDirs} SheenBidi/Headers)
set(sheenBidiPatterns SheenBidi/Source/*.c SheenBidi/Source/*.h SheenBidi/Headers/*.h)

file(GLOB_RECURSE externalSources ${sheenBidiPatterns})
# Don't care about warnings for external files.
if (MSVC)
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS /w)
else()
	set_source_files_properties(${externalSources} PROPERTIES COMPILE_FLAGS -w)
endif()

ds_glob_library_sources(sources Text src/*.c src/*.h include/*.h ${sheenBidiPatterns})
ds_add_library(deepsea_text Text ${sources})
ds_target_include_directories(deepsea_text PRIVATE ${textIncludeDirs})
ds_target_link_libraries(deepsea_text PRIVATE ${textLibraries})
ds_target_compile_definitions(deepsea_text PRIVATE ${textDefines})
