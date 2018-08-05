
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
        build:root( ('%s/lib'):format(variant) ),
    };
};

build:all {
    build:Executable 'hello_world' {
        '${lib}/hello_world';
        build:Cxx '${obj}/%1' {
            "executable.cpp"
        };
    };
};

build:Library '${lib}/hello_world' {
    build:Cxx '${obj}/%1' {
        "library.cpp"
    };
};
