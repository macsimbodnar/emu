@echo off

pushd "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build"
call vcvars64.bat
popd
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

set commonCompilerFlags=-MTd -Gm- -EHsc -WX -W4 -wd4838 -wd4702 -wd4701 -wd4244 -wd4244 -wd4267 -wd4996 -wd4201 -wd4100 -wd4505 -wd4189 -DHM_INTERNAL=1 -DHM_SLOW=1 -DHM_WIN32=1 -nologo -EHa- -GR- -Oi -FC -Z7 /I..\libs\windows\SDL2\include /I..\libs\windows\SDL2_ttf\include /I..\libs\SDL_FontCache
:: set commonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib
:: set commonDLLExport=/EXPORT:gameUpdateAndRender /EXPORT:gameGetSoundSamples

:: floating point optimisation -O2 -Oi -fp:fast 
:: ..\test_pixello.cpp ..\pixello.cpp
cl %commonCompilerFlags% ..\test_text.cpp ..\libs\SDL_FontCache\SDL_FontCache.c /link ..\libs\windows\SDL2\lib\x64\SDL2.lib ..\libs\windows\SDL2_ttf\lib\x64\SDL2_ttf.lib

:: xcopy /Y test_pixello.exe build\
xcopy /Y ..\libs\windows\SDL2\lib\x64\SDL2.dll .\
xcopy /Y ..\libs\windows\SDL2_ttf\lib\x64\SDL2_ttf.dll .\
xcopy /Y ..\libs\windows\SDL2_ttf\lib\x64\libfreetype-6.dll .\
xcopy /Y ..\libs\windows\SDL2_ttf\lib\x64\zlib1.dll .\

popd