:: windows release script
@echo off
setlocal

set ReleaseDir=x64\Release\

set LIBS=lib
set SDL2=%LIBS%\SDL2-2.26.4\lib\x64\SDL2.dll
set GLEW=%LIBS%\glew-2.2.0\bin\Release\x64\glew32.dll

for /f "delims=" %%i in ('git rev-parse --abbrev-ref HEAD') do set Branch=%%i
for /f "delims=" %%i in ('git rev-parse --short HEAD') do set Hash=%%i

set Release=flightsim-win32-x64.zip

:: copy DLLs
copy %SDL2% %ReleaseDir%
copy %GLEW% %ReleaseDir% 

:: write version info to file
(
  echo Release created: 
  echo %DATE% %TIME%
  echo Git Hash:
  git log -1 --pretty=oneline
) > %ReleaseDir%/VERSION.txt

del %Release%

:: compress 
tar.exe --exclude *.pdb -acvf %Release% -C %ReleaseDir% *

echo Current branch: %Branch%
echo Latest commit: %Hash%
