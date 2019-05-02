# https://pabloariasal.github.io/2018/02/19/its-time-to-do-cmake-right/

cmake_minimum_required(VERSION 3.8)
project(filter_spirit VERSION 0.0.0 LANGUAGES CXX)

##############################################################################
# options for customizing the build

option(FILTER_SPIRIT_BUILD_TESTS "Build unit tests" ON)

##############################################################################
# declare dependencies

find_package(Boost 1.68 REQUIRED
	COMPONENTS
		program_options
)
find_package(nlohmann_json 3.0.0 REQUIRED)
find_package(OpenSSL REQUIRED)

##############################################################################
# specify explicitly where to output all binary objects

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

##############################################################################
# create target and set its properties

# some source files are shared with unit test executable
# compile these as shared library so that it can be reused by both
# without the need to list and compile the same sources twice
add_library(filter_spirit_compiler SHARED
	src/fs/parser/parser.cpp
	src/fs/parser/print_error.cpp
	src/fs/parser/detail/grammar.cpp
	src/fs/compiler/compiler.cpp
	src/fs/compiler/process_input.cpp
	src/fs/compiler/print_error.cpp
	src/fs/compiler/detail/actions.cpp
	src/fs/compiler/detail/conditions.cpp
	src/fs/compiler/detail/convertions.cpp
	src/fs/compiler/detail/evaluate.cpp
	src/fs/compiler/detail/filter_builder.cpp
	src/fs/compiler/detail/functions.cpp
	src/fs/compiler/detail/queries.cpp
	src/fs/lang/action_set.cpp
	src/fs/lang/condition_set.cpp
	src/fs/lang/filter_block.cpp
	src/fs/lang/types.cpp
	src/fs/itemdata/parse_json.cpp
	src/fs/itemdata/types.cpp
	src/fs/log/buffered_logger.cpp
	src/fs/log/console_logger.cpp
	src/fs/log/logger.cpp
	src/fs/log/utility.cpp
	src/fs/utility/file.cpp
)

add_executable(filter_spirit
	src/fs/core/core.cpp
	src/fs/core/parse_args.cpp
	src/fs/main.cpp
	src/fs/network/network.cpp
	src/fs/network/poe_watch_api.cpp
	src/fs/network/session.cpp
)

target_include_directories(filter_spirit_compiler
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(filter_spirit
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

##############################################################################
# setup compiler flags

# Filter Spirit requires C++17
target_compile_features(filter_spirit_compiler
	PRIVATE
		cxx_std_17
)

target_compile_features(filter_spirit
	PRIVATE
		cxx_std_17
)

# add warnings

target_compile_options(filter_spirit_compiler
	PRIVATE
		$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_compile_options(filter_spirit
	PRIVATE
		$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

##############################################################################
# add libs that require linking and/or include paths

target_link_libraries(filter_spirit_compiler
	PRIVATE
		nlohmann_json::nlohmann_json
)

target_link_libraries(filter_spirit
	PRIVATE
		filter_spirit_compiler
		Boost::program_options
		nlohmann_json::nlohmann_json
		OpenSSL::SSL
)

# Windows requires additional networking libraries for Boost ASIO
if(WIN32)
	target_link_libraries(filter_spirit PRIVATE wsock32 ws2_32)
endif()

##############################################################################
# add directory with build recipe for tests

include(CTest) # adds option BUILD_TESTING (default ON)

if(BUILD_TESTING AND FILTER_SPIRIT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
