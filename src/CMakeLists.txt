add_subdirectory(lib)

if(FILTER_SPIRIT_BUILD_EXECUTABLE_CLI)
	add_subdirectory(cli)
endif()

if(FILTER_SPIRIT_BUILD_EXECUTABLE_GUI)
	add_subdirectory(gui)
endif()

include(CTest) # adds option BUILD_TESTING (default ON)
if(BUILD_TESTING AND FILTER_SPIRIT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
