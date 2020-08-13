if (ELEMENTS_LIBRARY_ROOT)
	add_subdirectory(${ELEMENTS_LIBRARY_ROOT} ${CMAKE_BINARY_DIR}/elements)
else()
	message(FATAL_ERROR "ELEMENTS_LIBRARY_ROOT is not set")
endif()

add_executable(filter_spirit_gui)

target_sources(filter_spirit_gui
	PRIVATE
		main.cpp
		ui.hpp
		ui.cpp
		user_state.hpp
		user_state.cpp
		gui_logger.hpp
		gui_logger.cpp
		item_preview.hpp
		item_preview.cpp
		platform/modal_dialogs.hpp
)

if(FILTER_SPIRIT_PLATFORM STREQUAL "win64")
	target_sources(filter_spirit_gui PRIVATE
		platform/windows/modal_dialogs.cpp
		platform/windows/string_utils.hpp
		platform/windows/string_utils.cpp
		platform/windows/generic_utils.hpp
		platform/windows/generic_utils.cpp
		platform/windows/com_utils.hpp)
elseif(FILTER_SPIRIT_PLATFORM STREQUAL "gtk")
	# (nothing currently)
else()
	message(FATAL_ERROR "missing or invalid FILTER_SPIRIT_PLATFORM")
endif()

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

if(FILTER_SPIRIT_ENABLE_LTO)
	set_target_properties(filter_spirit_gui PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_link_libraries(filter_spirit_gui
	PRIVATE
		filter_spirit
		cycfi::elements
)
