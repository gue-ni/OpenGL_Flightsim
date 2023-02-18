# OpenGL Flightsim

You can read an article about how it works [here](https://www.jakobmaier.at/posts/flight-simulation/).

![](OpenGL_Flightsim/reference/screencap/Flightsim_2023-02-18_09-09-41.gif)

## Build instructions (Windows)

To build this project we need [glm](https://github.com/g-truc/glm), [SDL2](https://www.libsdl.org/) and [GLEW](https://glew.sourceforge.net/). Download these libraries and put them in some location you can find again. After opening
the Visual Studio 2022 project go to the 'Property Pages' for the solution and set 'C/C++' -> 'General' -> 'Additional Include Directories' to your GLEW, SDL2 and glm include paths. Next we go to 'Linker' in the 'Property Pages' and set 'General' -> Additional Library Directories' to the appropriate paths.
Finally we set 'Linker' -> 'Input' -> 'Additional Dependencies' to include SDL2.lib, SDL2main.lib, glew32.lib and opengl32.lib. You may also have to set your environement variables to include 'opengl32.dll' and SDL2.dll.


