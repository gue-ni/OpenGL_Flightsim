:: windows release script
@echo off

set ReleaseDir=x64\Release\

set LIBS=lib
set SDL2=%LIBS%\SDL2-2.26.4\lib\x64\SDL2.dll
set GLEW=%LIBS%\glew-2.2.0\bin\Release\x64\glew32.dll

:: copy DLLs
copy %SDL2% %ReleaseDir%
copy %GLEW% %ReleaseDir% 

:: write version info to file
(
  echo %DATE% %TIME%
  git rev-parse HEAD
) > %ReleaseDir%/VERSION

del flightsim.zip

:: compress 
tar.exe --exclude *.pdb -acvf flightsim.zip -C %ReleaseDir% *
