rem @echo off

set SRC=%~dp0
set OBJ=%~dp0..\bootstrap-windows
set LIBS=%~dp0..\bootstrap-windows\lib
set BIN=%~dp0..\bootstrap-windows\bin

set DEFINES=/DBUILD_OS_WINDOWS /DBUILD_VARIANT_DEBUG /DBUILD_VERSION=\"bootstrap\" /DBOOST_ALL_NO_LIB
set INCLUDE_DIRS=/I %SRC% /I %SRC%lua /I %SRC%boost
set CC_FLAGS=%DEFINES% %INCLUDE_DIRS% /c /MTd /TC /Zi
set CXX_FLAGS=%DEFINES% %INCLUDE_DIRS% /c /MTd /TP /std:c++14 /EHsc /GR /Zi

if not exist "%OBJ%" mkdir %OBJ%
if not exist "%LIBS%" mkdir %LIBS%
echo boost_system & pushd boost\libs\system\src & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\boost_system.lib *.obj & popd
echo boost_filesystem & pushd boost\libs\filesystem\src & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\boost_filesystem.lib *.obj & popd
echo lua & pushd lua & cl.exe %CC_FLAGS% *.c & lib.exe /out:%LIBS%\lua.lib *.obj & popd
echo sweet/assert & pushd sweet\assert & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\assert.lib *.obj & popd
echo sweet/build_tool & pushd sweet\build_tool & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\build_tool.lib *.obj & popd
echo sweet/build_tool/build_tool_lua & pushd sweet\build_tool\build_tool_lua & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\build_tool_lua.lib *.obj & popd
echo sweet/cmdline & pushd sweet\cmdline & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\cmdline.lib *.obj & popd
echo sweet/error & pushd sweet\error & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\error.lib *.obj & popd
echo sweet/luaxx & pushd sweet\luaxx & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\luaxx.lib *.obj & popd
echo sweet/process & pushd sweet\process & cl.exe %CXX_FLAGS% *.cpp & lib.exe /out:%LIBS%\process.lib *.obj & popd

if not exist "%BIN%" mkdir %BIN%
set LIBRARIES=boost_system.lib boost_filesystem.lib lua.lib assert.lib build_tool.lib build_tool_lua.lib cmdline.lib error.lib luaxx.lib process.lib
echo sweet/build_tool/build & pushd sweet\build_tool\build & cl.exe %CXX_FLAGS% *.cpp & link /out:%BIN%\build.exe /libpath:%LIBS% /debug:full /pdb:%BIN%\build.pdb *.obj %LIBRARIES% & popd
