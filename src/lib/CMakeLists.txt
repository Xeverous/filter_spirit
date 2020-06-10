find_package(nlohmann_json 3.0.0 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost 1.68 REQUIRED COMPONENTS filesystem)

# libcurl >=7.17 has no requirements for string argument lifetimes
# FS relies on this in the implementation, so such version is a must
find_package(PkgConfig REQUIRED)
pkg_check_modules(libcurl REQUIRED IMPORTED_TARGET libcurl>=7.17.0)

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
		fs/compiler/outcome.cpp
		fs/compiler/detail/actions.cpp
		fs/compiler/detail/conditions.cpp
		fs/compiler/detail/evaluate.cpp
		fs/generator/generate_filter.cpp
		fs/generator/generator.cpp
		fs/generator/make_filter.cpp
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
		fs/network/url_encode.cpp
		fs/network/download.cpp
		fs/network/curl/error.cpp
		fs/network/poe_watch/download_data.cpp
		fs/network/poe_watch/parse_data.cpp
		fs/network/poe_watch/api_data.cpp
		fs/network/poe_ninja/download_data.cpp
		fs/network/poe_ninja/parse_data.cpp
		fs/network/poe_ninja/api_data.cpp
		fs/network/ggg/download_data.cpp
		fs/network/ggg/parse_data.cpp
	PUBLIC
		fs/compiler/detail/actions.hpp
		fs/compiler/detail/conditions.hpp
		fs/compiler/detail/evaluate.hpp
		fs/compiler/compiler.hpp
		fs/compiler/outcome.hpp
		fs/generator/make_filter.hpp
		fs/generator/generate_filter.hpp
		fs/generator/generator.hpp
		fs/generator/settings.hpp
		fs/lang/action_properties.hpp
		fs/lang/action_set.hpp
		fs/lang/condition_properties.hpp
		fs/lang/condition_set.hpp
		fs/lang/data_source_type.hpp
		fs/lang/item.hpp
		fs/lang/item_price_data.hpp
		fs/lang/item_filter.hpp
		fs/lang/item_classes.hpp
		fs/lang/keywords.hpp
		fs/lang/league.hpp
		fs/lang/limits.hpp
		fs/lang/object.hpp
		fs/lang/position_tag.hpp
		fs/lang/primitive_types.hpp
		fs/lang/queries.hpp
		fs/lang/symbol_table.hpp
		fs/log/string_logger.hpp
		fs/log/console_logger.hpp
		fs/log/logger.hpp
		fs/log/null_logger.hpp
		fs/log/structure_printer.hpp
		fs/network/url_encode.hpp
		fs/network/exceptions.hpp
		fs/network/download.hpp
		fs/network/curl/easy.hpp
		fs/network/curl/error.hpp
		fs/network/curl/libcurl.hpp
		fs/network/poe_ninja/api_data.hpp
		fs/network/poe_ninja/download_data.hpp
		fs/network/poe_ninja/parse_data.hpp
		fs/network/poe_watch/api_data.hpp
		fs/network/poe_watch/download_data.hpp
		fs/network/poe_watch/parse_data.hpp
		fs/network/ggg/api_data.hpp
		fs/network/ggg/download_data.hpp
		fs/network/ggg/parse_data.hpp
		fs/parser/ast.hpp
		fs/parser/ast_adapted.hpp
		fs/parser/detail/config.hpp
		fs/parser/detail/grammar.hpp
		fs/parser/detail/grammar_def.hpp
		fs/parser/detail/symbols.hpp
		fs/parser/error.hpp
		fs/parser/parser.hpp
		fs/utility/assert.hpp
		fs/utility/algorithm.hpp
		fs/utility/better_enum.hpp
		fs/utility/dump_json.hpp
		fs/utility/file.hpp
		fs/utility/holds_alternative.hpp
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

if(FILTER_SPIRIT_PLATFORM STREQUAL "win64")
	target_compile_definitions(filter_spirit PUBLIC FILTER_SPIRIT_PLATFORM_WINDOWS)
elseif(FILTER_SPIRIT_PLATFORM STREQUAL "gtk")
	target_compile_definitions(filter_spirit PUBLIC FILTER_SPIRIT_PLATFORM_GTK)
else()
	message(FATAL_ERROR "missing or invalid FILTER_SPIRIT_PLATFORM")
endif()

if(FILTER_SPIRIT_ENABLE_ASSERTION_EXCEPTIONS)
	target_compile_definitions(filter_spirit PRIVATE BOOST_ENABLE_ASSERT_HANDLER)
endif()

if(FILTER_SPIRIT_ENABLE_LTO)
	set_target_properties(filter_spirit PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# https://gitlab.kitware.com/cmake/cmake/issues/20431
get_target_property(CURL_INCLUDE_DIR PkgConfig::libcurl INTERFACE_INCLUDE_DIRECTORIES)
target_link_directories(filter_spirit
	PUBLIC
		${CURL_INCLUDE_DIR}/../bin)

target_link_libraries(filter_spirit
	PUBLIC
		nlohmann_json::nlohmann_json
		OpenSSL::SSL
		Boost::filesystem
		PkgConfig::libcurl
)

# FS uses atomic operations in networking callbacks
target_link_libraries(filter_spirit
	PUBLIC
		$<$<CXX_COMPILER_ID:GNU>:atomic>
		$<$<CXX_COMPILER_ID:Clang>:atomic>
)

# Windows requires additional networking libraries for Boost ASIO
if(WIN32)
	target_link_libraries(filter_spirit PRIVATE wsock32 ws2_32)
endif()
