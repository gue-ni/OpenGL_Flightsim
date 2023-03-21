cmake_minimum_required(VERSION 3.10)

project(OpenGL_Flightsim)

set(CMAKE_CXX_STANDARD 20)

find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(flightsim
    # Source files
    OpenGL_Flightsim/src/ai.h
    OpenGL_Flightsim/src/clipmap.h
    OpenGL_Flightsim/src/collisions.h
    OpenGL_Flightsim/src/data.h
    OpenGL_Flightsim/src/flightmodel.h
    OpenGL_Flightsim/src/gfx.cpp
    OpenGL_Flightsim/src/gfx.h
    OpenGL_Flightsim/src/main.cpp
    OpenGL_Flightsim/src/phi.h
    OpenGL_Flightsim/src/pid.h
    OpenGL_Flightsim/lib/stb_image.h
    OpenGL_Flightsim/lib/tiny_obj_loader.h
    OpenGL_Flightsim/lib/imgui/imconfig.h
    OpenGL_Flightsim/lib/imgui/imgui_demo.cpp
    OpenGL_Flightsim/lib/imgui/imgui.cpp
    OpenGL_Flightsim/lib/imgui/imgui_draw.cpp
    OpenGL_Flightsim/lib/imgui/imgui.h
    OpenGL_Flightsim/lib/imgui/imgui_impl_opengl3.cpp
    OpenGL_Flightsim/lib/imgui/imgui_impl_opengl3.h
    OpenGL_Flightsim/lib/imgui/imgui_impl_opengl3_loader.h
    OpenGL_Flightsim/lib/imgui/imgui_impl_sdl2.cpp
    OpenGL_Flightsim/lib/imgui/imgui_impl_sdl2.h
    OpenGL_Flightsim/lib/imgui/imgui_internal.h
    OpenGL_Flightsim/lib/imgui/imgui_tables.cpp
    OpenGL_Flightsim/lib/imgui/imgui_widgets.cpp
    OpenGL_Flightsim/lib/imgui/imstb_rectpack.h
    OpenGL_Flightsim/lib/imgui/imstb_textedit.h
    OpenGL_Flightsim/lib/imgui/imstb_truetype.h
)

target_include_directories(flightsim PUBLIC
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(flightsim
    SDL2
    GLEW
    GL
    GLU
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL_Flightsim/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL_Flightsim/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})