# find_package(nana REQUIRED)
if(NOT DEFINED NANA_REPO_DIR OR NOT DEFINED NANA_BUILD_DIR)
	message(SEND_ERROR "Define NANA_REPO_DIR to point to nana's GUI library repository and NANA_BUILD_DIR to nana's build directory")
else()
	add_subdirectory(${NANA_REPO_DIR} ${NANA_BUILD_DIR})
endif()

add_executable(filter_spirit_gui
	main.cpp
)

target_include_directories(filter_spirit_gui
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(filter_spirit_gui
	PRIVATE
		cxx_std_17
)

target_compile_options(filter_spirit_gui
	PRIVATE
		$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_link_libraries(filter_spirit_gui
	PRIVATE
		filter_spirit
		nana
)
