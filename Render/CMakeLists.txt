ds_add_module_library(deepsea_render deepsea_core deepsea_math deepsea_geometry)

if (NOT DEEPSEA_SYSTEM_MSL)
	set(externalPrefixes MSL)
endif()

# Only need to list it as a dependency if not a shared library.
if (NOT DEEPSEA_SHARED)
	set(externalDependencies MSLClient)
endif()

ds_install_library(TARGET deepsea_render MODULE Render DEPENDENCIES Core Math Geometry
	EXTERNAL_PREFIXES ${externalPrefixes} EXTERNAL_DEPENDENCIES ${externalDependencies})
set(DEEPSEA_DOC_PROJECTS ${DEEPSEA_DOC_PROJECTS} Render PARENT_SCOPE)
