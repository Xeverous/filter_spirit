# https://pabloariasal.github.io/2018/02/19/its-time-to-do-cmake-right/
# https://cliutils.gitlab.io/modern-cmake/

cmake_minimum_required(VERSION 3.8...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(filter_spirit
	VERSION 0.0.0
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
	"build Filter Spirit command line interface program" OFF)
option(FILTER_SPIRIT_BUILD_EXECUTABLE_GUI
	"build Filter Spirit graphic user interface program" ON)
option(FILTER_SPIRIT_BUILD_TESTS
	"build Filter Spirit tests" OFF)

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

# add our own cmake files to CMake module path - this will
# make CMake aware of our own FindLibraryName.cmake files
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_subdirectory(src)
