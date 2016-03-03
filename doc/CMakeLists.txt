find_package(Doxygen)
if (NOT DOXYGEN_FOUND)
	message("Doxygon not installed. Skipping documentation.")
	return()
endif()

set(rootDir ${CMAKE_CURRENT_SOURCE_DIR}/..)

foreach (docProject ${DEEPSEA_DOC_PROJECTS})
	set(thisDir ${rootDir}/${docProject}/include)
	set(sourceDirs ${sourceDirs} ${thisDir})
	file(GLOB_RECURSE theseDependencies ${thisDir}/*.h)
	set(dependencies ${dependencies} ${theseDependencies})
endforeach()

configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
set(outputFile ${CMAKE_CURRENT_BINARY_DIR}/html/index.html)
add_custom_command(OUTPUT ${outputFile}
	COMMAND ${DOXYGEN_EXECUTABLE} ARGS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS ${dependencies} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	COMMENT "Generating API documentation with Doxygen" VERBATIM)
add_custom_target(doc ALL DEPENDS ${outputFile})

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc/DeepSea)
