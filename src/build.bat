@echo off

REM NOTE(Justin): To compile demo.cpp, change main_file to demo.cpp.
set main_file=application.cpp
set common_compiler_flags=-Od -nologo -W4 -Zi -wd4100 -wd4005 -wd4201 -wd4800 -wd4244 -wd4459 -wd4505 -DGLEW_STATIC=1 -D_CRT_SECURE_NO_WARNINGS=1
set common_linker_flags=-incremental:no gdi32.lib user32.lib shell32.lib msvcrt.lib opengl32.lib winmm.lib glew32s.lib glfw3.lib assimp-vc142-mtd.lib
set include_directories= /I "../dependencies/assimp/include" /I "../dependencies/GLFW64/include" /I "../dependencies/GLEW/include" /I "../dependencies/glm"
set lib_directories= /LIBPATH:"../dependencies/GLFW64/lib-vc2019" /LIBPATH:"../dependencies/GLEW/lib/Release/x64" /LIBPATH:"../dependencies/assimp/lib/Debug" /LIBPATH:"../dependencies/assimp/bin/Debug"

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

cl ..\src\%main_file%  %common_compiler_flags% %include_directories%  /link /NODEFAULTLIB:"LIBCMT" %lib_directories% %common_linker_flags% 
popd

