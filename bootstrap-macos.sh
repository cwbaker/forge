
SRC=$(pwd)/src
LIB=$(pwd)/bootstrap-macos/lib
BIN=$(pwd)/bootstrap-macos/bin

cc() {
    for file in $1; do
        echo $file...
        local DEFINES="-DBUILD_OS_MACOS -DBUILD_VARIANT_DEBUG -DBUILD_VERSION=\"bootstrap\" -DLUA_USE_POSIX -DLUA_USE_DLOPEN"
        local INCLUDE_DIRS="-I $SRC -I $SRC/lua -I $SRC/boost"
        local FLAGS="-x c -g"
        clang $DEFINES $INCLUDE_DIRS $FLAGS -o $file.o -c $file
    done
}

cxx() {
    for file in $1; do
        echo $file...
        local DEFINES="-DBUILD_OS_MACOS -DBUILD_VARIANT_DEBUG -DBUILD_VERSION=\"bootstrap\" -DLUA_USE_POSIX -DLUA_USE_DLOPEN"
        local INCLUDE_DIRS="-I $SRC -I $SRC/lua -I $SRC/boost"
        local FLAGS="-x c++ -std=c++11 -stdlib=libc++ -fexceptions -frtti -g"
        clang++ $DEFINES $INCLUDE_DIRS $FLAGS -o $file.o -c $file
    done
}

archive() {
    ar -rcs $1 *.o    
}

link() {
    clang++ *.o -g -L $LIB -lassert -lcmdline -lbuild_tool -lbuild_tool_lua -lprocess -lluaxx -lerror -llua -lboost_filesystem -lboost_system -o $1
}

mkdir -p $LIB
echo boost_system; pushd $SRC/boost/libs/system/src; cxx "*.cpp"; archive $LIB/libboost_system.a; popd
echo boost_filesystem; pushd $SRC/boost/libs/filesystem/src; cxx "*.cpp"; archive $LIB/libboost_filesystem.a; popd
echo lua; pushd $SRC/lua; cc '*.c'; archive $LIB/liblua.a; popd
echo sweet/assert; pushd $SRC/sweet/assert; cxx '*.cpp'; archive $LIB/libassert.a; popd
echo sweet/build_tool; pushd $SRC/sweet/build_tool; cxx '*.cpp'; archive $LIB/libbuild_tool.a; popd
echo sweet/build_tool/build_tool_lua; pushd $SRC/sweet/build_tool/build_tool_lua; cxx '*.cpp'; archive $LIB/libbuild_tool_lua.a; popd
echo sweet/cmdline; pushd $SRC/sweet/cmdline; cxx '*.cpp'; archive $LIB/libcmdline.a; popd
echo sweet/error; pushd $SRC/sweet/error; cxx '*.cpp'; archive $LIB/liberror.a; popd
echo sweet/luaxx; pushd $SRC/sweet/luaxx; cxx '*.cpp'; archive $LIB/libluaxx.a; popd
echo sweet/process; pushd $SRC/sweet/process; cxx '*.cpp'; archive $LIB/libprocess.a; popd

mkdir -p $BIN
echo sweet/build_tool/build; pushd $SRC/sweet/build_tool/build; cxx '*.cpp'; link $BIN/build; popd
