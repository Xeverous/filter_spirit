find_package(nlohmann_json 3.0.0 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost 1.68 REQUIRED COMPONENTS filesystem)

# Some source files are shared with tests, GUI and CLI executables.
# Compile these as shared library so that it can be reused by both
# without the need to list and compile the same sources twice.
#
# We might also try an OBJECT target to reuse the compiled code
# directly - currently (pre-C++20 modules) it forces position-independent
# code on the whole content and the resulting shared library object is
# likely exporting a ton of unnecessary symbols. Object target could also
# speed up the linking because there will be much less exported symbols.
add_library(filter_spirit SHARED)

# We list all internal headers also as public because public headers
# include them indirectly, which still requires include path propagation
target_sources(filter_spirit
	PRIVATE
		fs/parser/parser.cpp
		fs/parser/detail/grammar.cpp
		fs/compiler/compiler.cpp
		fs/compiler/print_error.cpp
		fs/compiler/detail/actions.cpp
		fs/compiler/detail/conditions.cpp
		fs/compiler/detail/evaluate.cpp
		fs/generator/generate_filter.cpp
		fs/generator/generator.cpp
		fs/lang/action_set.cpp
		fs/lang/condition_set.cpp
		fs/lang/item_filter.cpp
		fs/lang/object.cpp
		fs/lang/item_price_data.cpp
		fs/lang/data_source_type.cpp
		fs/log/logger.cpp
		fs/utility/file.cpp
		fs/utility/dump_json.cpp
		fs/utility/string_helpers.cpp
		fs/network/http.cpp
		fs/network/poe_watch/download_data.cpp
		fs/network/poe_watch/parse_data.cpp
		fs/network/poe_watch/api_data.cpp
		fs/network/poe_ninja/download_data.cpp
		fs/network/poe_ninja/parse_data.cpp
		fs/network/poe_ninja/api_data.cpp
	PUBLIC
		fs/compiler/detail/actions.hpp
		fs/compiler/detail/conditions.hpp
		fs/compiler/detail/construct.hpp
		fs/compiler/detail/evaluate.hpp
		fs/compiler/detail/type_constructors.hpp
		fs/compiler/compiler.hpp
		fs/compiler/error.hpp
		fs/compiler/print_error.hpp
		fs/generator/generate_filter.hpp
		fs/generator/generator.hpp
		fs/generator/options.hpp
		fs/lang/action_properties.hpp
		fs/lang/action_set.hpp
		fs/lang/condition_properties.hpp
		fs/lang/condition_set.hpp
		fs/lang/data_source_type.hpp
		fs/lang/item.hpp
		fs/lang/item_price_data.hpp
		fs/lang/item_filter.hpp
		fs/lang/keywords.hpp
		fs/lang/league.hpp
		fs/lang/object.hpp
		fs/lang/position_tag.hpp
		fs/lang/price_range.hpp
		fs/lang/primitive_types.hpp
		fs/lang/queries.hpp
		fs/lang/symbol_table.hpp
		fs/lang/traits/is_lang_type.hpp
		fs/lang/traits/promotions.hpp
		fs/log/string_logger.hpp
		fs/log/console_logger.hpp
		fs/log/logger.hpp
		fs/log/null_logger.hpp
		fs/log/structure_printer.hpp
		fs/network/async_download.hpp
		fs/network/exceptions.hpp
		fs/network/http.hpp
		fs/network/poe_ninja/api_data.hpp
		fs/network/poe_ninja/download_data.hpp
		fs/network/poe_ninja/parse_data.hpp
		fs/network/poe_watch/api_data.hpp
		fs/network/poe_watch/download_data.hpp
		fs/network/poe_watch/parse_data.hpp
		fs/parser/ast.hpp
		fs/parser/ast_adapted.hpp
		fs/parser/detail/config.hpp
		fs/parser/detail/grammar.hpp
		fs/parser/detail/grammar_def.hpp
		fs/parser/detail/symbols.hpp
		fs/parser/error.hpp
		fs/parser/parser.hpp
		fs/utility/algorithm.hpp
		fs/utility/better_enum.hpp
		fs/utility/dump_json.hpp
		fs/utility/file.hpp
		fs/utility/holds_alternative.hpp
		fs/utility/type_list.hpp
		fs/utility/type_name.hpp
		fs/utility/type_traits.hpp
		fs/utility/visitor.hpp
		fs/utility/string_helpers.hpp
		fs/version.hpp
)

target_include_directories(filter_spirit
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

# Filter Spirit requires C++17
target_compile_features(filter_spirit
	PUBLIC
		cxx_std_17
)

# we add -Wno-deprecated-copy for GCC due to a false positive
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=92145
target_compile_options(filter_spirit
	PRIVATE
		$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Wno-deprecated-copy -ffast-math>
		$<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_link_libraries(filter_spirit
	PRIVATE
		nlohmann_json::nlohmann_json
		OpenSSL::SSL
		Boost::filesystem
)

# Windows requires additional networking libraries for Boost ASIO
if(WIN32)
	target_link_libraries(filter_spirit PRIVATE wsock32 ws2_32)
endif()
