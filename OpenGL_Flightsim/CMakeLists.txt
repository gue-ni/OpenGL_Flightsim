cmake_minimum_required(VERSION 3.10)


project(OpenGL_Flightsim)

set(CMAKE_CXX_STANDARD 20)

find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(OpenGL_Flightsim-app 
    # Source files
    src/ai.h
    src/clipmap.h
    src/collisions.h
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

target_include_directories(OpenGL_Flightsim-app PUBLIC 
/usr/include/SDL2/
)

target_link_libraries(OpenGL_Flightsim-app
    SDL2 
    GLEW 
    GL 
    GLU
)
