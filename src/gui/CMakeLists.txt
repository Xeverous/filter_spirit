if (ELEMENTS_LIBRARY_ROOT)
	add_subdirectory(${ELEMENTS_LIBRARY_ROOT} ${CMAKE_BINARY_DIR}/elements)
else()
	message(FATAL_ERROR "ELEMENTS_LIBRARY_ROOT is not set")
endif()

add_executable(filter_spirit_gui)

target_sources(filter_spirit_gui
	PRIVATE
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
		$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -ffast-math -save-temps>
		$<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

if(FILTER_SPIRIT_ENABLE_LTO)
	set_target_properties(filter_spirit_gui PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_link_libraries(filter_spirit_gui
	PRIVATE
		filter_spirit
		cycfi::elements
)
