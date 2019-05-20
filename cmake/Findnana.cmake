# Find the nana library
#
# This will define the following variables
#
#    NANA_FOUND
#    NANA_INCLUDE_DIRS
#
# and the following imported targets
#
#     nana::nana

find_package(PkgConfig)
pkg_check_modules(PC_NANA QUIET nana)

find_package(PNG REQUIRED)
message("PNG_PNG_INCLUDE_DIR = ${PNG_PNG_INCLUDE_DIR}")
message("PNG_LIBRARY = ${PNG_LIBRARY}")

find_path(NANA_INCLUDE_DIR
    NAMES gui.hpp
    PATHS ${PC_NANA_INCLUDE_DIRS} ${NANA_ROOT}
    PATH_SUFFIXES include/nana
)

find_library(NANA_LIBRARY
    NAMES nana
	PATHS ${PC_NANA_LIBRARY_DIRS} ${NANA_ROOT}
    PATH_SUFFIXES lib bin
)

set(NANA_VERSION ${PC_NANA_VERSION})

mark_as_advanced(NANA_FOUND NANA_INCLUDE_DIR NANA_LIBRARY NANA_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nana
    REQUIRED_VARS NANA_INCLUDE_DIR NANA_LIBRARY
    VERSION_VAR NANA_VERSION
)

if(NANA_FOUND)
    #Set include dirs to parent, to enable includes like #include <libname/file.hpp>
    get_filename_component(NANA_INCLUDE_DIRS ${NANA_INCLUDE_DIR} DIRECTORY)
endif()

if(NANA_FOUND AND NOT TARGET nana::nana)
	message("found nana library, include path(s): ${NANA_INCLUDE_DIRS}, library file: ${NANA_LIBRARY}")
    add_library(nana::nana STATIC IMPORTED GLOBAL)
    set_target_properties(nana::nana PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${NANA_INCLUDE_DIRS}"
		IMPORTED_LOCATION "${NANA_LIBRARY}"
		INTERFACE_LINK_LIBRARIES png
    )
endif()
