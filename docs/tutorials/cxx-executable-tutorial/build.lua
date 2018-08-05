
require 'build';
require 'build.cc';
require 'build.linux';
require 'build.macos';
require 'build.windows';

variant = build:lower( variant or 'debug' );

build:initialize {
    variants = { "debug", "release", "shipping" };
    bin = build:root( ('%s/bin'):format(variant) );
    lib = build:root( ('%s/lib'):format(variant) );
    obj = build:root( ('%s/obj'):format(variant) );
    include_directories = {
        build:root( 'src' )
    };
    library_directories = {
    };
};

build:all {
    build:Executable 'hello_world' {
        build:Cxx '${obj}/%1' {
            'hello_world.cpp'
        };
    };
};
