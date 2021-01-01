# declare dependencies

find_package(Boost 1.68 REQUIRED)

##############################################################################
# create target and set its properties

add_executable(filter_spirit_test)

target_sources(filter_spirit_test
	PRIVATE
		main.cpp
		parser/lookup.cpp
		parser/parser_tests.cpp
		parser/parser_error_tests.cpp
		compiler/compiler_error_tests.cpp
		compiler/filter_generation_tests.cpp
		compiler/compiler_tests.cpp
		lang/pass_item_through_filter_tests.cpp
		utility/algorithm_tests.cpp
		utility/string_helpers_tests.cpp
		common/test_fixtures.cpp
		common/string_operations.cpp
		common/print_type.hpp
		common/string_operations.hpp
		common/test_fixtures.hpp
)

target_include_directories(filter_spirit_test
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

##############################################################################
# setup compiler flags

# Filter Spirit requires C++17
target_compile_features(filter_spirit_test
	PRIVATE
		cxx_std_17
)

# add warnings if supported
target_compile_options(filter_spirit_test
	PRIVATE
		$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

##############################################################################
# add libs that require linking and/or include paths

target_link_libraries(filter_spirit_test PRIVATE filter_spirit)

##############################################################################
# register test executable for CTest

add_test(NAME unit_tests COMMAND filter_spirit_test)
