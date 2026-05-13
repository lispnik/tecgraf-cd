# CD Library Comprehensive Test Suite

cmake_minimum_required(VERSION 3.16)

# Test utilities library
add_library(cd_test_utils STATIC
    utils/test_utils.c
)
target_include_directories(cd_test_utils PUBLIC
    utils
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(cd_test_utils PUBLIC cd)

# Function to create a test executable
function(add_cd_test test_name source_file)
    add_executable(${test_name} ${source_file})
    target_link_libraries(${test_name} PRIVATE cd_test_utils cd)
    target_include_directories(${test_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        utils
    )

    # Create output directory for this test
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output/${test_name})

    # Add test to CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output/${test_name}
    )
endfunction()

# Core functionality tests
add_cd_test(test_primitives tests/test_primitives.c)
add_cd_test(test_attributes tests/test_attributes.c)
add_cd_test(test_text tests/test_text.c)
add_cd_test(test_images tests/test_images.c)
add_cd_test(test_transforms tests/test_transforms.c)
add_cd_test(test_clipping tests/test_clipping.c)
add_cd_test(test_paths tests/test_paths.c)
add_cd_test(test_colors tests/test_colors.c)

# Backend-specific tests
add_cd_test(test_backends tests/test_backends.c)
add_cd_test(test_svg tests/test_svg.c)
add_cd_test(test_irgb tests/test_irgb.c)

# Integration tests
add_cd_test(test_comprehensive tests/test_comprehensive.c)
add_cd_test(test_edge_cases tests/test_edge_cases.c)
add_cd_test(test_performance tests/test_performance.c)

# Regression tests for known issues
add_cd_test(test_regression tests/test_regression.c)

# Create a master test runner
add_executable(run_all_tests tests/run_all_tests.c)
target_link_libraries(run_all_tests PRIVATE cd_test_utils cd)

# Legacy basic test for compatibility
add_executable(cd_basic_test basic_test.c)
target_link_libraries(cd_basic_test PRIVATE cd)
add_test(NAME basic_test COMMAND cd_basic_test)

# Platform-specific backend tests (disabled until implemented)
# if(CD_ENABLE_IM AND IM_FOUND)
#     add_cd_test(test_im_integration tests/test_im_integration.c)
#     target_link_libraries(test_im_integration PRIVATE ${IM_LIBRARY})
#     target_include_directories(test_im_integration PRIVATE ${IM_INCLUDE_DIR})
# endif()

# TODO: Add platform-specific backend tests
# if(CD_PLATFORM_LINUX AND X11_FOUND)
#     add_cd_test(test_x11_backend tests/test_x11_backend.c)
# endif()

# if(CD_ENABLE_CAIRO AND CAIRO_FOUND)
#     add_cd_test(test_cairo_backend tests/test_cairo_backend.c)
# endif()

# if(CD_ENABLE_GL AND OPENGL_FOUND)
#     add_cd_test(test_opengl_backend tests/test_opengl_backend.c)
# endif()

# Custom targets
add_custom_target(test_cd
    COMMAND ${CMAKE_COMMAND} -E echo "Running CD Test Suite..."
    COMMAND run_all_tests
    DEPENDS run_all_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(test_cd_visual
    COMMAND ${CMAKE_COMMAND} -E echo "Running visual tests..."
    COMMAND test_primitives
    COMMAND test_attributes
    COMMAND test_text
    COMMAND test_svg
    DEPENDS test_primitives test_attributes test_text test_svg
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Install test data files if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data)
    install(DIRECTORY data/ DESTINATION share/cd/test_data)
endif()