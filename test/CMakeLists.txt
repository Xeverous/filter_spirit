# declare dependencies

find_package(Boost 1.68 REQUIRED
	COMPONENTS
		unit_test_framework
)
find_package(nlohmann_json 3.0.0 REQUIRED)

##############################################################################
# create target and set its properties

add_executable(filter_spirit_test
	fsut/main.cpp
	fsut/parser/positive.cpp
	fsut/utility/algorithm.cpp
)

target_include_directories(filter_spirit_test
	PRIVATE
		${CMAKE_SOURCE_DIR}/src
		${CMAKE_SOURCE_DIR}/test
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
		$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -fvisibility=hidden -ffast-math>
		$<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic -fvisibility=hidden -ffast-math>
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

##############################################################################
# add libs that require linking and/or include paths

target_link_libraries(filter_spirit_test
	PRIVATE
		filter_spirit_compiler
		Boost::unit_test_framework
		nlohmann_json::nlohmann_json
)

##############################################################################
# register test executable for CTest

add_test(NAME unit_tests COMMAND filter_spirit_test)
