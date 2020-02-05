# CMake 3.11 or above is required due to the use of the module 'FetchContent'
cmake_minimum_required(VERSION 3.14)
project(FIB)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    message(STATUS "Setting C++ standard to C++11, since none was specified")
    set(CMAKE_CXX_STANDARD 17)
endif()

# GLFW
set(GLFW_BUILD_EXAMPLES FALSE CACHE BOOL "Overwrite GLFW default value" FORCE)
set(GLFW_BUILD_TESTS FALSE CACHE BOOL "Overwrite GLFW default value" FORCE)
set(GLFW_BUILD_DOCS FALSE CACHE BOOL "Overwrite GLFW default value" FORCE)
set(GLFW_INSTALL FALSE CACHE BOOL "Overwrite GLFW default value" FORCE)
set(glfw_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/glfw)
set(glfw_BINARY_DIR ${CMAKE_BINARY_DIR}/3rdparty/glfw)
add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})

# Dear ImGui
set(imgui_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/imgui)
set(DEP_SRC
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/examples/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/examples/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/examples/libs/gl3w/GL/gl3w.c
)

# Main application
add_executable(fibui
    ${DEP_SRC}
    src/main.cpp
)

target_include_directories(fibui
    PRIVATE
    # imgui
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/examples
    ${imgui_SOURCE_DIR}/examples/libs/gl3w

    # include
    include
)

if(WIN32)
    set(NATIVE_LIBRARIES opengl32.lib gdi32.lib shell32.lib)
elseif(APPLE)
    set(NATIVE_LIBRARIES "-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo")
endif()

target_link_libraries(fibui PRIVATE glfw ${NATIVE_LIBRARIES})

