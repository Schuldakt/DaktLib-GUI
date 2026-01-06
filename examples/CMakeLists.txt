cmake_minimum_required(VERSION 3.23)

# ============================================================================
# DaktLib-GUI Examples
# ============================================================================

option(DAKTLIBGUI_BUILD_EXAMPLES "Build example applications" OFF)

if(DAKTLIBGUI_BUILD_EXAMPLES)
    # Find GLFW for windowing
    find_package(glfw3 3.3 QUIET)
    find_package(OpenGL REQUIRED)
    
    if(glfw3_FOUND)
        message(STATUS "GLFW found - building showcase example")
        
        add_executable(DaktLib-GUI_showcase showcase.cpp)
        
        target_link_libraries(DaktLib-GUI_showcase PRIVATE 
            $<IF:$<TARGET_EXISTS:DaktLib-GUI_static>,DaktLib-GUI_static,DaktLib-GUI_shared>
            glfw
            OpenGL::GL
        )
        
        target_compile_features(DaktLib-GUI_showcase PRIVATE cxx_std_23)
        
        # Copy to build directory
        set_target_properties(DaktLib-GUI_showcase PROPERTIES
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
