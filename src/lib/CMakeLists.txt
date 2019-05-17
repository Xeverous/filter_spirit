find_package(nlohmann_json 3.0.0 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost 1.68 REQUIRED)

# some source files are shared with tests and GUI/CLI executables
# compile these as shared library so that it can be reused by both
# without the need to list and compile the same sources twice
add_library(filter_spirit SHARED
	fs/parser/parser.cpp
	fs/parser/print_error.cpp
	fs/parser/detail/grammar.cpp
	fs/compiler/compiler.cpp
	fs/compiler/print_error.cpp
	fs/compiler/detail/actions.cpp
	fs/compiler/detail/conditions.cpp
	fs/compiler/detail/convertions.cpp
	fs/compiler/detail/evaluate.cpp
	fs/compiler/detail/filter_builder.cpp
	fs/compiler/detail/queries.cpp
	fs/compiler/detail/determine_types_of.cpp
	fs/generator/generate_filter.cpp
	fs/generator/generator.cpp
	fs/lang/action_set.cpp
	fs/lang/condition_set.cpp
	fs/lang/filter_block.cpp
	fs/lang/types.cpp
	fs/lang/item_price_data.cpp
	fs/log/buffered_logger.cpp
	fs/log/console_logger.cpp
	fs/log/logger.cpp
	fs/log/utility.cpp
	fs/utility/file.cpp
	fs/network/network.cpp
	fs/network/session.cpp
	fs/network/poe_watch/api.cpp
	fs/network/poe_watch/parse_json.cpp
	fs/core/core.cpp
)

target_include_directories(filter_spirit
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

# Filter Spirit requires C++17
target_compile_features(filter_spirit
	PRIVATE
		cxx_std_17
)

target_compile_options(filter_spirit
	PRIVATE
		$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_link_libraries(filter_spirit
	PRIVATE
		nlohmann_json::nlohmann_json
		OpenSSL::SSL
)

# Windows requires additional networking libraries for Boost ASIO
if(WIN32)
	target_link_libraries(filter_spirit PRIVATE wsock32 ws2_32)
endif()

set_target_properties(filter_spirit PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
