find_package(Freetype REQUIRED)
find_package(HarfBuzz REQUIRED)

file(GLOB_RECURSE sources src/*.c src/*.h include/*.h include/*.mslh)
file(GLOB_RECURSE externalSources SheenBidi/Source/*.c SheenBidi/Source/*.h SheenBidi/Headers/*.h)
ds_add_library(deepsea_text MODULE Text FILES ${sources} EXTERNAL_FILES ${externalSources}
	DEPENDS deepsea_render)

ds_target_include_directories(deepsea_text PRIVATE
	${FREETYPE_INCLUDE_DIRS} ${HARFBUZZ_INCLUDE_DIRS} SheenBidi/Headers)
ds_target_link_libraries(deepsea_text PRIVATE ${FREETYPE_LIBRARIES} ${HARFBUZZ_LIBRARIES})

ds_install_library(TARGET deepsea_text MODULE Text DEPENDENCIES Render)

add_subdirectory(test)
