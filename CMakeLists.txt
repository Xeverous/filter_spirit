# https://pabloariasal.github.io/2018/02/19/its-time-to-do-cmake-right/
# https://cliutils.gitlab.io/modern-cmake/

# 3.13 is required for:
# - relative source file paths
# - target_link_directories()
# - INTERFACE_LINK_LIBRARIES
cmake_minimum_required(VERSION 3.13...3.16)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(filter_spirit
	VERSION 0.5.0
	DESCRIPTION "advanced item filter generator for the game Path of Exile using own DSL and online APIs"
	LANGUAGES CXX)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
		STRING "Choose the type of build." FORCE)
	# Set the possible values of build type for cmake-gui
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
		"Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

##############################################################################
# options for customizing the build

option(FILTER_SPIRIT_BUILD_EXECUTABLE_CLI
	"build Filter Spirit command line program" ON)
option(FILTER_SPIRIT_BUILD_TESTS
	"build Filter Spirit tests" ON)
option(FILTER_SPIRIT_ENABLE_ASSERTION_EXCEPTIONS
	"ON: throw on assertion failure; OFF: rely on assert() behavior" OFF)

##############################################################################
# specify explicitly where to output all binary objects

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

if(supported)
    message(STATUS "IPO / LTO supported, enabling")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO / LTO not supported: <${error}>, will not enable")
endif()

##############################################################################

add_subdirectory(src)
