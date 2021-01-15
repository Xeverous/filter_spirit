set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})

# Add Corrade as a subproject
set(CORRADE_USE_PEDANTIC_FLAGS ON)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/corrade" corrade EXCLUDE_FROM_ALL)

# Add Magnum as a subproject, enable specific application implementation
if(FILTER_SPIRIT_PLATFORM STREQUAL "emscripten")
	set(WITH_EMSCRIPTENAPPLICATION ON CACHE BOOL "" FORCE)
else()
	set(WITH_SDL2APPLICATION ON CACHE BOOL "" FORCE)
endif()

add_subdirectory("${CMAKE_SOURCE_DIR}/external/magnum" magnum EXCLUDE_FROM_ALL)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(WITH_IMGUI ON CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/magnum-integration" magnum-integration EXCLUDE_FROM_ALL)

add_subdirectory("${CMAKE_SOURCE_DIR}/external/tiny_file_dialogs" tiny_file_dialogs EXCLUDE_FROM_ALL)

if(FILTER_SPIRIT_PLATFORM STREQUAL "emscripten")
	find_package(Magnum REQUIRED GL EmscriptenApplication)
else()
	find_package(Magnum REQUIRED GL Sdl2Application)
endif()

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
		fs/gui/auxiliary/file_dialogs.cpp
		fs/gui/auxiliary/file_dialogs.hpp
		fs/gui/auxiliary/raii.hpp
		fs/gui/auxiliary/rect.hpp
		fs/gui/auxiliary/widgets.cpp
		fs/gui/auxiliary/widgets.hpp
		fs/gui/settings/fonting.cpp
		fs/gui/settings/fonting.hpp
		fs/gui/settings/theming.cpp
		fs/gui/settings/theming.hpp
		fs/gui/settings/networking.cpp
		fs/gui/settings/networking.hpp
		fs/gui/windows/about_window.cpp
		fs/gui/windows/about_window.hpp
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
		fs/gui/windows/filter/market_data_state.cpp
		fs/gui/windows/filter/market_data_state.hpp
		fs/gui/windows/filter/filter_state_mediator.cpp
		fs/gui/windows/filter/filter_state_mediator.hpp
		fs/gui/windows/filter/real_filter_state_mediator.cpp
		fs/gui/windows/filter/real_filter_state_mediator.hpp
		fs/gui/windows/filter/spirit_filter_state_mediator.cpp
		fs/gui/windows/filter/spirit_filter_state_mediator.hpp
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
		Corrade::Main
		MagnumIntegration::ImGui
		Magnum::Application
		Magnum::GL
		Magnum::Magnum
)

if(FILTER_SPIRIT_PLATFORM STREQUAL "emscripten")
	target_compile_options(filter_spirit_gui PUBLIC
		-fexceptions
		"SHELL: -s DISABLE_EXCEPTION_CATCHING=0" # we need exceptions (too much code uses them)
		"SHELL: -s WASM=0"
	)

	target_link_options(filter_spirit_gui PUBLIC
		-fexceptions
		"SHELL: -s USE_SDL=2"   # use emscripten's port of SDLv2 library
		"SHELL: -s STRICT=1"    # disable deprecated back-compat emscripten behavior
		"SHELL: -s STRICT_JS=1" # Add "use strict;" to generated JS
		"SHELL: -s INITIAL_MEMORY=134217728 -s ALLOW_MEMORY_GROWTH=1"
		"SHELL: -s DISABLE_EXCEPTION_CATCHING=0"
		"SHELL: -s WASM=0"
		"SHELL: --preload-file ${CMAKE_SOURCE_DIR}/data@data --preload-file ${CMAKE_SOURCE_DIR}/assets@assets"
	)

	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(filter_spirit_gui PUBLIC
			"SHELL: -s EXCEPTION_DEBUG=1"
		)

		target_link_options(filter_spirit_gui PUBLIC
			"SHELL: -s EXCEPTION_DEBUG=1"
		)
	endif()

	add_custom_target(copy_page ALL
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/emscripten/index.html
		${CMAKE_BINARY_DIR}/bin
	)

	add_dependencies(filter_spirit_gui copy_page)
else()
	target_link_libraries(filter_spirit_gui PRIVATE tiny_file_dialogs)

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

	add_custom_target(copy_certificates ALL
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/certificates
		${CMAKE_BINARY_DIR}/bin/certificates
	)

	add_dependencies(filter_spirit_gui copy_data copy_assets copy_certificates)
endif()

