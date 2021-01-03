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
	VERSION 0.7.0
	DESCRIPTION "advanced item filter generator for the game Path of Exile using own DSL and online APIs"
	LANGUAGES C CXX)

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
option(FILTER_SPIRIT_BUILD_EXECUTABLE_GUI
	"build Filter Spirit graphic user interface program" ON)
option(FILTER_SPIRIT_BUILD_TESTS
	"build Filter Spirit tests" ON)
option(FILTER_SPIRIT_ENABLE_ASSERTION_EXCEPTIONS
	"ON: throw on assertion failure; OFF: rely on assert() behavior" OFF)
option(FILTER_SPIRIT_ADD_FILESYSTEM_LINK
	"add linker command for standard filesystem library (required by earlier GCC and Clang versions)" OFF)
option(FILTER_SPIRIT_ENABLE_LTO
	"enable link-time optimization for Filter Spirit targets" ON)
option(FILTER_SPIRIT_USE_EXTERNAL_JSON
	"ON: use installed nlohmann/json library through find_package(); OFF: use from repository submodule" OFF)
set(FILTER_SPIRIT_PLATFORM "" CACHE STRING
	"win7 (Windows 7 or newer) OR sdl2 (any platform suppored by SDL2) OR emscripten")

##############################################################################
# specify explicitly where to output all binary objects

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# disable checking LTO support on emscripten, its broken
if(FILTER_SPIRIT_ENABLE_LTO AND NOT FILTER_SPIRIT_PLATFORM STREQUAL "emscripten")
	include(CheckIPOSupported)
	check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)

	if(IPO_SUPPORTED)
		message(STATUS "IPO / LTO supported, enabling")
	else()
		message(FATAL_ERROR "IPO / LTO not supported: ${IPO_ERROR}, change settings or compiler")
	endif()
endif()
##############################################################################

add_subdirectory(src)
