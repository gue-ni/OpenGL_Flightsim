@echo on

set LibDir=lib
mkdir %LibDir%

:: SDL2
curl -fsSL -o SDL2.zip https://github.com/libsdl-org/SDL/releases/download/release-2.26.4/SDL2-devel-2.26.4-VC.zip
tar -xf SDL2.zip
move SDL2-2.26.4 %LibDir%
del SDL2.zip

:: glm
curl -fsSL -o glm.zip https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip
tar -xf glm.zip 
move glm %LibDir%
del glm.zip

:: glew
curl -fsSL -o glew.zip https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip
tar -xf glew.zip
move glew-2.2.0 %LibDir%
del glew.zip
