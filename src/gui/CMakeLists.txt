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
		imgui_window.hpp
		ui_utils.hpp
		filter_template_window.hpp
		filter_template_window.cpp
		real_filter_window.hpp
		real_filter_window.cpp
		color_picker_window.hpp
		color_picker_window.cpp
		single_item_preview_window.hpp
		single_item_preview_window.cpp
		common_ui_settings_window.hpp
		common_ui_settings_window.cpp
		version_info_window.hpp
		version_info_window.cpp
		application.hpp
		application.cpp
		theming.hpp
		theming.cpp
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
