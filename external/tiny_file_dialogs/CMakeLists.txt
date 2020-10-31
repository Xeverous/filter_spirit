add_library(tiny_file_dialogs STATIC)

target_sources(tiny_file_dialogs
	PRIVATE
		tinyfiledialogs-code/tinyfiledialogs.c
	PUBLIC
		tinyfiledialogs-code/tinyfiledialogs.h
)

target_include_directories(tiny_file_dialogs
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/tinyfiledialogs-code
)

target_compile_features(tiny_file_dialogs PRIVATE c_std_11)

target_compile_options(tiny_file_dialogs
	PRIVATE
		$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

if(FILTER_SPIRIT_ENABLE_LTO)
	set_target_properties(tiny_file_dialogs PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(FILTER_SPIRIT_PLATFORM STREQUAL "win7")
	target_link_libraries(tiny_file_dialogs PUBLIC Comdlg32 Ole32)
endif()
