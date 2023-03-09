# OpenGL Flight Simulator

You can read an article about how it works [here](https://www.jakobmaier.at/posts/flight-simulation/). There is also a compiled demo available.

https://user-images.githubusercontent.com/45669207/221434341-9068181d-0ff3-401a-bf34-29b530a92154.mp4

## Controls

The demo supports input with a keyboard and a joystick. With a keyboard, use WASD to control pitch and roll and F and K to increase and decrease thrust. E and Q control the rudder. You can use P to pause and O to toggle the camera.

## Build instructions (Windows)

To build this project we need [glm](https://github.com/g-truc/glm), [SDL2](https://www.libsdl.org/) and [GLEW](https://glew.sourceforge.net/). Download these libraries and put them in some location you can find again. As you will see, I put them in 'libraries'. After opening
the Visual Studio 2022 project go to the 'Property Pages' for the solution and set 'C/C++' -> 'General' -> 'Additional Include Directories' to your GLEW, SDL2 and glm include paths. Next we go to 'Linker' in the 'Property Pages' and set 'General' -> Additional Library Directories' to the appropriate paths.
Finally we set 'Linker' -> 'Input' -> 'Additional Dependencies' to include SDL2.lib, SDL2main.lib, glew32.lib and opengl32.lib. You may also have to set your environement variables to include 'opengl32.dll' and SDL2.dll.


## Build instructions (Linux)

Install dependencies SDL2, GLEW and OpenGL and build using cmake.

```
$ apt install libsdl2-dev libsdl2-image-dev libglew-dev libgle3-dev cmake
$ mkdir build 
$ cd build
$ cmake ..
$ cmake --build .
```