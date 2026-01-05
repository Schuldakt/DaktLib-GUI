# Phase 1 Unit Tests
add_executable(phase1_tests_exe phase1_tests.cpp)

target_include_directories(phase1_tests_exe PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(phase1_tests_exe PRIVATE
    DaktLib-GUI
)

add_test(NAME Phase1Tests COMMAND phase1_tests_exe)
