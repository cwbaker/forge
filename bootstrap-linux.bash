
SRC=$(pwd)/src
LIB=$(pwd)/bootstrap/lib
BIN=$(pwd)/bootstrap/bin
AR=${AR:-ar}
CC=${CC:-gcc}
CXX=${CXX:-g++}

cc() {
    for file in $1; do
        echo $file...
        local DEFINES="-DBUILD_VARIANT_DEBUG -DBUILD_VERSION=\"bootstrap\" -DLUA_USE_POSIX -DLUA_USE_DLOPEN"
        local INCLUDE_DIRS="-I $SRC -I $SRC/lua/src -I $SRC/boost"
        local FLAGS="-x c -g"
        $CC $DEFINES $INCLUDE_DIRS $FLAGS -o $file.o -c $file
    done
}

cxx() {
    for file in $1; do
        echo $file...
        local DEFINES="-DBUILD_VARIANT_DEBUG -DBUILD_VERSION=\"bootstrap\" -DLUA_USE_POSIX -DLUA_USE_DLOPEN"
        local INCLUDE_DIRS="-I $SRC -I $SRC/lua/src -I $SRC/boost"
        local FLAGS="-x c++ -std=c++11 -fexceptions -frtti -fPIC -march=native -g -Wno-deprecated-declarations"
        $CXX $DEFINES $INCLUDE_DIRS $FLAGS -o $file.o -c $file
    done
}

archive() {
    $AR -rcs $1 *.o    
}

link_forge() {
    $CXX *.o -g -L $LIB -lforge -lforge_lua -lprocess -lcmdline -lluaxx -lerror -llua -lassert -lboost_filesystem -lboost_system -lpthread -ldl -o $BIN/forge
}

link_forge_hooks() {
    $CXX *.o -shared -g -o $BIN/libforge_hooks.so
}

mkdir -p $LIB
echo boost_system; pushd $SRC/boost/libs/system/src; cxx "*.cpp"; archive $LIB/libboost_system.a; popd
echo boost_filesystem; pushd $SRC/boost/libs/filesystem/src; cxx "*.cpp"; archive $LIB/libboost_filesystem.a; popd
echo lua; pushd $SRC/lua/src; cc '*.c'; archive $LIB/liblua.a; popd
echo assert; pushd $SRC/assert; cxx '*.cpp'; archive $LIB/libassert.a; popd
echo forge; pushd $SRC/forge; cxx '*.cpp'; archive $LIB/libforge.a; popd
echo forge/forge_lua; pushd $SRC/forge/forge_lua; cxx '*.cpp'; archive $LIB/libforge_lua.a; popd
echo cmdline; pushd $SRC/cmdline; cxx '*.cpp'; archive $LIB/libcmdline.a; popd
echo error; pushd $SRC/error; cxx '*.cpp'; archive $LIB/liberror.a; popd
echo luaxx; pushd $SRC/luaxx; cxx '*.cpp'; archive $LIB/libluaxx.a; popd
echo process; pushd $SRC/process; cxx '*.cpp'; archive $LIB/libprocess.a; popd

mkdir -p $BIN
echo forge/forge; pushd $SRC/forge/forge; cxx '*.cpp'; link_forge; popd
echo forge/forge_hooks; pushd $SRC/forge/forge_hooks; cxx 'forge_hooks_linux.cpp'; link_forge_hooks; popd
