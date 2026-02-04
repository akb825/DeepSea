find_package(Freetype QUIET)
find_package(HarfBuzz QUIET)
if (NOT FREETYPE_FOUND OR NOT HARFBUZZ_FOUND)
	message("Freetype or HarfBuzz not found. Skipping Text.")
	return()
endif()

# Link to zlib in case FreeType is statically linked.
find_package(ZLIB QUIET)
set(extraLibraries)
if (ZLIB_FOUND)
	set(extraLibraries ${ZLIB_LIBRARIES})
endif()

file(GLOB_RECURSE sources src/*.c src/*.h include/*.h include/*.mslh)
file(GLOB_RECURSE externalSources SheenBidi/Source/*.c SheenBidi/Source/*.h SheenBidi/Headers/*.h)
ds_add_library(deepsea_text MODULE Text FILES ${sources} EXTERNAL_FILES ${externalSources})

# Static HarfBuzz library requires C++ linkage
if (NOT DEEPSEA_SINGLE_SHARED)
	set_target_properties(deepsea_text PROPERTIES LINKER_LANGUAGE CXX)
endif()

ds_target_include_directories(deepsea_text PRIVATE
	${FREETYPE_INCLUDE_DIRS} ${HARFBUZZ_INCLUDE_DIRS} SheenBidi/Headers SheenBidi/Source)
ds_target_link_libraries(deepsea_text PRIVATE ${HARFBUZZ_LIBRARIES} ${FREETYPE_LIBRARIES}
	${extraLibraries})

ds_install_library(TARGET deepsea_text MODULE Text DEPENDS Render)

add_subdirectory(test)
