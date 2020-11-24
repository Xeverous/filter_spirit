set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})

# Add Corrade as a subproject
set(CORRADE_USE_PEDANTIC_FLAGS ON)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/corrade" corrade EXCLUDE_FROM_ALL)

# Add Magnum as a subproject, enable Sdl2Application
set(WITH_SDL2APPLICATION ON CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/magnum" magnum EXCLUDE_FROM_ALL)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(WITH_IMGUI ON CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/magnum-integration" magnum-integration EXCLUDE_FROM_ALL)

add_subdirectory("${CMAKE_SOURCE_DIR}/external/tiny_file_dialogs" tiny_file_dialogs EXCLUDE_FROM_ALL)

find_package(Magnum REQUIRED GL Sdl2Application)
find_package(MagnumIntegration REQUIRED ImGui)

add_executable(filter_spirit_gui)

target_sources(filter_spirit_gui
	PRIVATE
		fs/gui/application.cpp
		fs/gui/application.hpp
		fs/gui/gui_logger.cpp
		fs/gui/gui_logger.hpp
		fs/gui/imgui_window.hpp
		fs/gui/auxiliary/color_convert.hpp
		fs/gui/auxiliary/colors.hpp
		fs/gui/auxiliary/raii.hpp
		fs/gui/auxiliary/rect.hpp
		fs/gui/auxiliary/widgets.cpp
		fs/gui/auxiliary/widgets.hpp
		fs/gui/settings/fonting.cpp
		fs/gui/settings/fonting.hpp
		fs/gui/settings/theming.cpp
		fs/gui/settings/theming.hpp
		fs/gui/windows/application_log_window.cpp
		fs/gui/windows/application_log_window.hpp
		fs/gui/windows/color_picker_window.cpp
		fs/gui/windows/color_picker_window.hpp
		fs/gui/windows/settings_window.cpp
		fs/gui/windows/settings_window.hpp
		fs/gui/windows/filter_windows.cpp
		fs/gui/windows/filter_windows.hpp
		fs/gui/windows/filter/debug_state.cpp
		fs/gui/windows/filter/debug_state.cpp
		fs/gui/windows/filter/item_tooltip.cpp
		fs/gui/windows/filter/item_tooltip.hpp
		fs/gui/windows/filter/filter_state_mediator.cpp
		fs/gui/windows/filter/filter_state_mediator.hpp
		fs/gui/windows/filter/filter_state_mediator_base.hpp
		fs/gui/windows/filter/real_filter_state_mediator.cpp
		fs/gui/windows/filter/real_filter_state_mediator.hpp
		fs/gui/windows/filter/real_filter_state_mediator_base.hpp
		fs/gui/windows/filter/spirit_filter_state_mediator.cpp
		fs/gui/windows/filter/spirit_filter_state_mediator.hpp
		fs/gui/windows/filter/spirit_filter_state_mediator_base.hpp
		fs/gui/windows/filter/loot_state.cpp
		fs/gui/windows/filter/loot_state.hpp
		fs/gui/windows/filter/source_state.cpp
		fs/gui/windows/filter/source_state.hpp
		fs/gui/windows/single_item_preview_window.cpp
		fs/gui/windows/single_item_preview_window.hpp
		fs/gui/windows/version_info_window.cpp
		fs/gui/windows/version_info_window.hpp
		fs/main.cpp
)

target_include_directories(filter_spirit_gui
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(filter_spirit_gui
	PRIVATE
		cxx_std_17
)

# we add -Wno-deprecated-copy for GCC due to a false positive
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=92145
target_compile_options(filter_spirit_gui
	PRIVATE
		$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Wno-deprecated-copy -ffast-math>
		$<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic -ffast-math>
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

if(FILTER_SPIRIT_ENABLE_LTO)
	set_target_properties(filter_spirit_gui PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_link_libraries(filter_spirit_gui
	PRIVATE
		filter_spirit
		tiny_file_dialogs
		MagnumIntegration::ImGui
		Magnum::Application
		Magnum::GL
		Magnum::Magnum
)

add_custom_target(copy_data ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/data
	${CMAKE_BINARY_DIR}/bin/data
)

add_custom_target(copy_assets ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/assets
	${CMAKE_BINARY_DIR}/bin/assets
)

add_dependencies(filter_spirit_gui copy_data copy_assets)
