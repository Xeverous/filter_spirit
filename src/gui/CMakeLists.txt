set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})

# Add Corrade as a subproject
set(CORRADE_USE_PEDANTIC_FLAGS ON)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/corrade" corrade EXCLUDE_FROM_ALL)

# Add Magnum as a subproject, enable Sdl2Application
set(WITH_SDL2APPLICATION ON CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/magnum" magnum EXCLUDE_FROM_ALL)

find_package(Magnum REQUIRED GL Sdl2Application)

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
		Magnum::Application
		Magnum::GL
		Magnum::Magnum
)
