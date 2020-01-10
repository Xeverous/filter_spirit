# declare dependencies

find_package(Boost 1.68 REQUIRED
	COMPONENTS
		unit_test_framework
)

##############################################################################
# create target and set its properties

add_executable(filter_spirit_test)

target_sources(filter_spirit_test
	PRIVATE
		fst/main.cpp
		fst/parser/parser_tests.cpp
		fst/parser/parser_error_tests.cpp
		fst/compiler/compiler_error_tests.cpp
		fst/compiler/filter_generation_tests.cpp
		fst/compiler/compiler_tests.cpp
		fst/common/test_fixtures.cpp
		fst/common/string_operations.cpp
		fst/utility/algorithm_tests.cpp
		fst/common/print_type.hpp
		fst/common/string_operations.hpp
		fst/common/test_fixtures.hpp
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

target_link_libraries(filter_spirit_test
	PRIVATE
		filter_spirit
		Boost::unit_test_framework
)

##############################################################################
# register test executable for CTest

add_test(NAME unit_tests COMMAND filter_spirit_test)
