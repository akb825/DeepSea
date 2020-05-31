if (NOT DOXYGEN_FOUND OR NOT DEEPSEA_BUILD_DOCS)
	return()
endif()

set(docSource "\"${DEEPSEA_SOURCE_DIR}/README.md\"")
set(dependencies ${DEEPSEA_SOURCE_DIR}/README.md)
get_property(modules GLOBAL PROPERTY DEEPSEA_MODULE_PATHS)
foreach (docProject ${modules})
	set(thisDir ${DEEPSEA_MODULE_DIR}/${docProject})
	file(GLOB mdFiles ${thisDir}/*.md)
	set(quotedMDFiles)
	foreach (mdFile ${mdFiles})
		set(quotedMDFiles "${quotedMDFiles} \"${mdFile}\"")
	endforeach()
	set(docSource "${docSource} \"${thisDir}/include\" ${quotedMDFiles}")
	set(includeDirs "${includeDirs} \"${thisDir}/include\"")
	file(GLOB_RECURSE theseDependencies ${thisDir}/include/*.h)
	set(dependencies ${dependencies} ${theseDependencies} ${mdFiles})
endforeach()

# Remove all generated files.
set(removeItems)
foreach (file ${dependencies})
	if (file MATCHES ".*_generated\.h")
		list(APPEND removeItems ${file})
	endif()
endforeach()

if (removeItems)
	list(REMOVE_ITEM dependencies ${removeItems})
endif()

configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
set(outputFile ${CMAKE_CURRENT_BINARY_DIR}/html/index.html)
add_custom_command(OUTPUT ${outputFile}
	COMMAND ${DOXYGEN_EXECUTABLE} ARGS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${DEEPSEA_SOURCE_DIR}
	DEPENDS ${dependencies} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	COMMENT "Generating API documentation with Doxygen" VERBATIM)
add_custom_target(doc ALL DEPENDS ${outputFile})

ds_set_folder(doc "")

if (DEEPSEA_INSTALL)
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc/DeepSea)
endif()
