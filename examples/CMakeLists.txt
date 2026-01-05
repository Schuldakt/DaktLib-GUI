cmake_minimum_required(VERSION 3.23)

# ============================================================================
# DaktLib-GUI Examples
# ============================================================================

option(DAKTGUI_BUILD_EXAMPLES "Build example applications" OFF)

if(DAKTGUI_BUILD_EXAMPLES)
    # Find GLFW for windowing
    find_package(glfw3 3.3 QUIET)
    find_package(OpenGL REQUIRED)
    
    if(glfw3_FOUND)
        message(STATUS "GLFW found - building showcase example")
        
        add_executable(daktgui_showcase showcase.cpp)
        
        target_link_libraries(daktgui_showcase PRIVATE 
            $<IF:$<TARGET_EXISTS:daktgui_static>,daktgui_static,daktgui_shared>
            glfw
            OpenGL::GL
        )
        
        target_compile_features(daktgui_showcase PRIVATE cxx_std_23)
        
        # Copy to build directory
        set_target_properties(daktgui_showcase PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )
    else()
        message(STATUS "GLFW not found - skipping showcase example")
        message(STATUS "  To build examples, install GLFW:")
        message(STATUS "    macOS:  brew install glfw")
        message(STATUS "    Ubuntu: apt install libglfw3-dev")
        message(STATUS "    Windows: vcpkg install glfw3")
    endif()
endif()
