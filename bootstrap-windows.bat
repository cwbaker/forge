rem @echo off

echo Running bootstrap-windows.bat...
for /f "usebackq delims=#" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere" -latest -property installationPath`) do call "%%i\VC\Auxiliary\Build\vcvars64.bat"

set SRC=%~dp0src
set OBJ=%~dp0bootstrap
set LIBS=%~dp0bootstrap\lib
set BIN=%~dp0bootstrap\bin

set DEFINES=/DBUILD_VARIANT_DEBUG /DBUILD_VERSION=\"bootstrap\" /DBOOST_ALL_NO_LIB /D_WIN32_WINNT=0x0a00
set INCLUDE_DIRS=/I %SRC% /I %SRC%\lua\src /I %SRC%\boost
set CC_FLAGS=%DEFINES% %INCLUDE_DIRS% /c /MTd /TC /Zi
set CXX_FLAGS=%DEFINES% %INCLUDE_DIRS% /c /MTd /TP /std:c++14 /EHsc /GR /Zi

if not exist "%OBJ%" mkdir %OBJ%
if not exist "%LIBS%" mkdir %LIBS%
echo boost_system & pushd %SRC%\boost\libs\system\src & del *.obj & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\boost_system.lib *.obj & popd
echo boost_filesystem & pushd %SRC%\boost\libs\filesystem\src & del *.obj & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\boost_filesystem.lib *.obj & popd
echo lua & pushd %SRC%\lua\src & del *.obj & cl.exe %CC_FLAGS% *.c & lib.exe /out:%LIBS%\lua.lib *.obj & popd
echo assert & pushd %SRC%\assert & del *.obj & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\assert.lib *.obj & popd
echo forge & pushd %SRC%\forge & del *.obj & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\forge.lib *.obj & popd
echo forge/forge_lua & pushd %SRC%\forge\forge_lua & del *.obj & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\forge_lua.lib *.obj & popd
echo cmdline & pushd %SRC%\cmdline & del *.obj & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\cmdline.lib *.obj & popd
echo error & pushd %SRC%\error & del *.obj & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\error.lib *.obj & popd
echo luaxx & pushd %SRC%\luaxx & del *.obj & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\luaxx.lib *.obj & popd
echo process & pushd %SRC%\process & del *.obj & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\process.lib *.obj & popd

if not exist "%BIN%" mkdir %BIN%
set LIBRARIES=boost_system.lib boost_filesystem.lib lua.lib assert.lib forge.lib forge_lua.lib cmdline.lib error.lib luaxx.lib process.lib
echo forge/forge & pushd %SRC%\forge\forge & del *.obj & cl.exe %CXX_FLAGS% *.cpp & link /out:%BIN%\forge.exe /libpath:%LIBS% /debug:full /pdb:%BIN%\forge.pdb *.obj %LIBRARIES% & popd
echo forge/forge_hooks & pushd %SRC%\forge\forge_hooks & del *.obj & cl.exe %CXX_FLAGS% forge_hooks_windows.cpp ImportDescriptor.cpp & link /dll /out:%BIN%\forge_hooks.dll /libpath:%LIBS% /debug:full /pdb:%BIN%\forge_hooks.pdb *.obj assert.lib & popd
