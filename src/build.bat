@echo off

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -Od -Oi -WX -W4 -wd4100 -wd4189 -wd4201 -wd4457 -wd4505 -DAPP_INTERNAL=1 -DAPP_SLOW=1 -DAPP_WIN32=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib opengl32.lib

IF NOT EXIST ..\..\build mkdir ..\..\build 
pushd ..\..\build

REM 64-bit build
del *.pdb > NUL 2> NUL
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% ..\opengl\src\app.cpp -Fmapp.map -LD /link -incremental:no -opt:ref -PDB:app_%random%.pdb -EXPORT:AppGetSoundSamples -EXPORT:AppUpdateAndRender
del lock.tmp
cl %CommonCompilerFlags% ..\opengl\src\win32_app.cpp -Fmwin32_app.map /link %CommonLinkerFlags%
popd



