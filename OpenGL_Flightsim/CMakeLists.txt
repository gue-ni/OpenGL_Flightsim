cmake_minimum_required(VERSION 3.10)

project(OpenGL_Flightsim)

set(CMAKE_CXX_STANDARD 20)

find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(flightsim
    # Source files
    src/ai.h
    src/clipmap.h
    src/collider.h
    src/data.h
    src/flightmodel.h
    src/gfx.cpp
    src/gfx.h
    src/main.cpp
    src/phi.h
    src/pid.h
    lib/stb_image.h
    lib/tiny_obj_loader.h
    lib/imgui/imconfig.h
    lib/imgui/imgui_demo.cpp
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui.h
    lib/imgui/imgui_impl_opengl3.cpp
    lib/imgui/imgui_impl_opengl3.h
    lib/imgui/imgui_impl_opengl3_loader.h
    lib/imgui/imgui_impl_sdl2.cpp
    lib/imgui/imgui_impl_sdl2.h
    lib/imgui/imgui_internal.h
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imstb_rectpack.h
    lib/imgui/imstb_textedit.h
    lib/imgui/imstb_truetype.h
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

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
