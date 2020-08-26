@echo off

:: "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

IF NOT EXIST build mkdir build
pushd build
:: -WX consider warnings like error so don't compile
:: -W4 level 4 warnings
:: -wd[numer] exclude [number] worning from compilation
:: -EHsc or -EHa- disable exception catch for warning C4530 
:: -Zi produce .pdb files for debuger. We will use -Z7 
:: -Oi for debug system call
:: -GR- disable runtime information

:: 4189 warning unused wariables 

set commonCompilerFlags=-MTd -Gm- -EHsc -WX -W4 -wd4201 -wd4100 -wd4505 -wd4189 -DHM_INTERNAL=1 -DHM_SLOW=1 -DHM_WIN32=1 -nologo -EHa- -GR- -Oi -FC -Z7 /I..\libs\windows\SDL\include
:: set commonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib
:: set commonDLLExport=/EXPORT:gameUpdateAndRender /EXPORT:gameGetSoundSamples

:: floating point optimisation -O2 -Oi -fp:fast 
cl %commonCompilerFlags% ..\test_pixello.cpp ..\pixello.cpp /link ..\libs\windows\SDL\lib\x64\SDL2.lib

:: xcopy /Y test_pixello.exe build\
xcopy /Y ..\libs\windows\SDL\lib\x64\SDL2.dll .\

popd