# https://pabloariasal.github.io/2018/02/19/its-time-to-do-cmake-right/

cmake_minimum_required(VERSION 3.8)
project(filter_spirit VERSION 0.0.0 LANGUAGES CXX)

##############################################################################
# declare dependencies

find_package(Boost 1.68 REQUIRED COMPONENTS
	program_options
)
find_package(nlohmann_json 3.0.0 REQUIRED)
find_package(OpenSSL REQUIRED)

##############################################################################
# create target and set its properties

add_executable(filter_spirit
	src/compiler/actions.cpp
	src/compiler/compiler.cpp
	src/compiler/conditions.cpp
	src/compiler/convertions.cpp
	src/compiler/evaluate.cpp
	src/compiler/filter_builder.cpp
	src/compiler/functions.cpp
	src/compiler/queries.cpp
	src/core/core.cpp
	src/core/parse_args.cpp
	src/itemdata/parse_json.cpp
	src/itemdata/types.cpp
	src/lang/action_set.cpp
	src/lang/condition_set.cpp
	src/lang/filter_block.cpp
	src/lang/types.cpp
	src/log/buffered_logger.cpp
	src/log/console_logger.cpp
	src/log/logger.cpp
	src/main.cpp
	src/network/network.cpp
	src/network/poe_watch_api.cpp
	src/network/session.cpp
	src/parser/grammar.cpp
	src/parser/parser.cpp
	src/print/compile_error.cpp
	src/print/generic.cpp
	src/print/parse_error.cpp
	src/utility/file.cpp
)

target_include_directories(filter_spirit
# we do not export any public interface
#	PUBLIC
# add required top-level directory of our own includes
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

##############################################################################
# setup compiler flags

# Filter Spirit requires C++17
target_compile_features(filter_spirit
	PRIVATE
		cxx_std_17
)

# add warnings if supported
target_compile_options(filter_spirit
	PRIVATE
		$<$<CXX_COMPILER_ID:GNU>:-Wall>
		$<$<CXX_COMPILER_ID:GNU>:-Wextra>
		$<$<CXX_COMPILER_ID:GNU>:-Wpedantic>
)

##############################################################################
# add libs that require linking and/or include paths

target_link_libraries(filter_spirit
	PRIVATE
		Boost::program_options
		nlohmann_json::nlohmann_json
		OpenSSL::SSL
)

# Windows requires additional networking libraries for Boost ASIO
if(WIN32)
	target_link_libraries(filter_spirit PRIVATE wsock32 ws2_32)
endif()
