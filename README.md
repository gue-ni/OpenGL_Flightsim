# OpenGL Sandbox

## Setup OpenGL with GLFW

[Video](https://www.youtube.com/watch?v=HzFatL3WT6g&ab_channel=BoostMyTool)

1.	Download GLFW precompiled binary from [here](https://www.glfw.org/download.html)
2.	Save to somewhere like `C:\User\jakob\libraries`
3.	In Visual Studio, go to *Project* -> *Properties* 
	-	Add the path of the *include* directory to *C/C++* -> *General* -> *Additional Include Directories*
	-	Add the path of *lib-vc2022* to *Linker* -> *General* -> *Additional Library Directories*
	-	Add *glfw3.lib; opengl32.lib; user32.lib; gdi32.lib; shell32.lib* to *Linker* -> *Input* -> *Additional Dependencies*
3. Download GLAD from [here](https://glad.dav1d.de/)
	-	Add the path of the *glad/include* directory to *C/C++* -> *General* -> *Additional Include Directories*
  - Add *glad.c* to sources


## Setup OpenGL with SDL2







