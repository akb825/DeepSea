file(GLOB_RECURSE sources src/*.c src/*.h include/*.h)

if (UNIX AND NOT APPLE)
	find_package(Wayland QUIET)
	if (WAYLAND_FOUND)
		if (WAYLAND_SCANNER_FOUND)
			file(GLOB protocols src/Platform/*.xml)
			set(generatedSources)
			foreach (protocolXML ${protocols})
				get_filename_component(protocol ${protocolXML} NAME_WE)
				set(protocolHeader ${CMAKE_CURRENT_BINARY_DIR}/include/${protocol}-protocol.h)
				set(protocolSrc ${CMAKE_CURRENT_BINARY_DIR}/${protocol}-protocol.c)
				add_custom_command(OUTPUT ${protocolHeader} COMMAND ${WAYLAND_SCANNER}
					ARGS client-header ${protocolXML} ${protocolHeader}
					DEPENDS ${WAYLAND_SCANNER} ${protocolXML}
					COMMENT "Generating Wayland protocol header: ${protocolHeader}")
				add_custom_command(OUTPUT ${protocolSrc} COMMAND ${WAYLAND_SCANNER}
					ARGS private-code ${protocolXML} ${protocolSrc}
					DEPENDS ${WAYLAND_SCANNER} ${protocolXML}
					COMMENT "Generating Wayland protocol source: ${protocolSrc}")
				list(APPEND generatedSources ${protocolHeader} ${protocolSrc})
			endforeach()
			ds_generated_sources(deepsea_application_wayland_protocols ${generatedSources})
			list(APPEND sources ${generatedSources})
		else()
			message(WARNING "Wayland found, but wayland-scanner not found; skipping support for Wayland color manager.")
		endif()
	endif()
endif()

ds_add_library(deepsea_application MODULE Application FILES ${sources})
ds_target_include_directories(deepsea_application PRIVATE src)

if (WAYLAND_FOUND AND WAYLAND_SCANNER_FOUND)
	ds_target_include_directories(deepsea_application PRIVATE ${WAYLAND_INCLUDE_DIR})
	ds_target_compile_definitions(deepsea_application
		PUBLIC DS_APPLICATION_HAS_WAYLAND_COLOR_MANAGER=1)
else()
	ds_target_compile_definitions(deepsea_application
		PUBLIC DS_APPLICATION_HAS_WAYLAND_COLOR_MANAGER=0)
endif()

if (WIN32)
	ds_target_link_libraries(deepsea_application PRIVATE dxgi.lib)
endif()

ds_install_library(TARGET deepsea_application MODULE Application DEPENDS Render)
