cmake_minimum_required(VERSION 3.2)

if (DEFINED ENV{TRAVIS_BUILD_NUMBER})
    project(sparse-matrix VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})
else()
    project(sparse-matrix VERSION 0.0.1)
endif()

find_package(GTest REQUIRED)

add_executable(sparse-matrix-main src/main.cpp src/sparse_matrix.h)
add_executable(sparse-matrix-test src/tests.cpp)

set_target_properties(sparse-matrix-main sparse-matrix-test PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
        COMPILE_OPTIONS -Wpedantic -Wall -Wextra)

include_directories(${GTEST_INCLUDE_DIRS})
target_link_libraries(sparse-matrix-test ${GTEST_LIBRARIES} pthread)

install(TARGETS sparse-matrix-main RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT yukigaru@gmail.com)

include(CPack)

enable_testing()
add_test(sparse-matrix-test sparse-matrix-test)

