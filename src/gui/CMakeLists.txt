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
		fs/gui/imgui_window.hpp
		fs/gui/ui_utils.hpp
		fs/gui/windows/color_picker_window.cpp
		fs/gui/windows/color_picker_window.hpp
		fs/gui/windows/common_ui_settings_window.cpp
		fs/gui/windows/common_ui_settings_window.hpp
		fs/gui/windows/filter_template_window.cpp
		fs/gui/windows/filter_template_window.hpp
		fs/gui/windows/real_filter_window.cpp
		fs/gui/windows/real_filter_window.hpp
		fs/gui/windows/settings/fonting.cpp
		fs/gui/windows/settings/fonting.hpp
		fs/gui/windows/settings/theming.cpp
		fs/gui/windows/settings/theming.hpp
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
