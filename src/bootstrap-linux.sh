
SRC=$(pwd)
LIB=$(pwd)/../bootstrap/lib
BIN=$(pwd)/../bootstrap/bin

cc() {
    for file in $1; do
        echo $file...
        local DEFINES="-DBUILD_OS_LINUX -DBUILD_VARIANT_DEBUG -DBUILD_VERSION=\"bootstrap\" -DLUA_USE_POSIX -DLUA_USE_DLOPEN"
        local INCLUDE_DIRS="-I $SRC -I $SRC/lua -I $SRC/boost"
        local FLAGS="-x c -g"
        gcc $DEFINES $INCLUDE_DIRS $FLAGS -o $file.o -c $file
    done
}

cxx() {
    for file in $1; do
        echo $file...
        local DEFINES="-DBUILD_OS_LINUX -DBUILD_VARIANT_DEBUG -DBUILD_VERSION=\"bootstrap\" -DLUA_USE_POSIX -DLUA_USE_DLOPEN"
        local INCLUDE_DIRS="-I $SRC -I $SRC/lua -I $SRC/boost"
        local FLAGS="-x c++ -std=c++11 -fexceptions -frtti -g -Wno-deprecated-declarations"
        g++ $DEFINES $INCLUDE_DIRS $FLAGS -o $file.o -c $file
    done
}

archive() {
    ar -rcs $1 *.o    
}

link() {
    g++ *.o -g -L $LIB -lbuild_tool -lbuild_tool_lua -lprocess -lcmdline -lsweetlua -lerror -llua -lassert -lpersist -lrtti -lboost_filesystem -lboost_system -lpthread -ldl -o $1
}

mkdir -p $LIB
echo boost_system; pushd boost/libs/system/src; cxx "*.cpp"; archive $LIB/libboost_system.a; popd
echo boost_filesystem; pushd boost/libs/filesystem/src; cxx "*.cpp"; archive $LIB/libboost_filesystem.a; popd
echo lua; pushd lua; cc '*.c'; archive $LIB/liblua.a; popd
echo sweet/assert; pushd sweet/assert; cxx '*.cpp'; archive $LIB/libassert.a; popd
echo sweet/build_tool; pushd sweet/build_tool; cxx '*.cpp'; archive $LIB/libbuild_tool.a; popd
echo sweet/build_tool/build_tool_lua; pushd sweet/build_tool/build_tool_lua; cxx '*.cpp'; archive $LIB/libbuild_tool_lua.a; popd
echo sweet/cmdline; pushd sweet/cmdline; cxx '*.cpp'; archive $LIB/libcmdline.a; popd
echo sweet/error; pushd sweet/error; cxx '*.cpp'; archive $LIB/liberror.a; popd
echo sweet/lua; pushd sweet/lua; cxx '*.cpp'; archive $LIB/libsweetlua.a; popd
echo sweet/persist; pushd sweet/persist; cxx '*.cpp'; archive $LIB/libpersist.a; popd
echo sweet/process; pushd sweet/process; cxx '*.cpp'; archive $LIB/libprocess.a; popd
echo sweet/rtti; pushd sweet/rtti; cxx '*.cpp'; archive $LIB/librtti.a; popd

mkdir -p $BIN
echo sweet/build_tool/build; pushd sweet/build_tool/build; cxx '*.cpp'; link $BIN/build; popd
