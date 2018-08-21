
require 'forge';
require 'forge.cc';
require 'forge.linux';
require 'forge.macos';
require 'forge.windows';

variant = forge:lower( variant or 'debug' );

forge:initialize {
    variants = { "debug", "release", "shipping" };
    bin = forge:root( ('%s/bin'):format(variant) );
    lib = forge:root( ('%s/lib'):format(variant) );
    obj = forge:root( ('%s/obj'):format(variant) );
    include_directories = {
        forge:root( 'src' )
    };
    library_directories = {
    };
};

forge:all {
    forge:Executable 'hello_world' {
        forge:Cxx '${obj}/%1' {
            'hello_world.cpp'
        };
    };
};
