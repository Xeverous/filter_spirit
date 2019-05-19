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

find_path(NANA_INCLUDE_DIR
    NAMES gui.hpp
    PATHS ${PC_NANA_INCLUDE_DIRS}
    PATH_SUFFIXES nana
)

set(NANA_VERSION ${PC_NANA_VERSION})

mark_as_advanced(NANA_FOUND NANA_INCLUDE_DIR NANA_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nana
    REQUIRED_VARS NANA_INCLUDE_DIR
    VERSION_VAR NANA_VERSION
)

if(NANA_FOUND)
    #Set include dirs to parent, to enable includes like #include <libname/file.hpp>
    get_filename_component(NANA_INCLUDE_DIRS ${NANA_INCLUDE_DIR} DIRECTORY)
endif()

if(NANA_FOUND AND NOT TARGET nana::nana)
    add_library(nana::nana INTERFACE IMPORTED)
    set_target_properties(nana::nana PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${NANA_INCLUDE_DIRS}"
    )
endif()