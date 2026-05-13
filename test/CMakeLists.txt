cmake_minimum_required(VERSION 3.16)

# Find test dependencies
find_package(PkgConfig)

# Basic test that just verifies the library can be linked
add_executable(cd_basic_test basic_test.c)
target_link_libraries(cd_basic_test PRIVATE cd)
add_test(NAME basic_test COMMAND cd_basic_test)

# Test canvas creation if we have appropriate backends
if(CD_ENABLE_IM AND IM_FOUND)
    add_executable(cd_image_test image_test.c)
    target_link_libraries(cd_image_test PRIVATE cd)
    target_include_directories(cd_image_test PRIVATE ${IM_INCLUDE_DIR})
    add_test(NAME image_test COMMAND cd_image_test)
endif()

# Platform-specific tests
if(CD_PLATFORM_LINUX AND X11_FOUND)
    add_executable(cd_x11_test x11_test.c)
    target_link_libraries(cd_x11_test PRIVATE cd ${X11_LIBRARIES})
    target_include_directories(cd_x11_test PRIVATE ${X11_INCLUDE_DIR})
    add_test(NAME x11_test COMMAND cd_x11_test)
endif()

# Test suite targets
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS cd_basic_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)