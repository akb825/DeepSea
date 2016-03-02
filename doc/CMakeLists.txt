find_package(Doxygen)
if (NOT DOXYGEN_FOUND)
	message("Doxygon not installed. Skipping documentation.")
	return()
endif()

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc ALL
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating API documentation with Doxygen" VERBATIM
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc/DeepSea)
