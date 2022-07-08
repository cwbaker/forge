
variant = variant or 'debug';

local forge = require( 'forge' ):load( variant );

local cc = forge.Toolset 'cc_${platform}_${architecture}' {
    platform = operating_system();
    architecture = 'x86-64';
    bin = root( ('%s/bin'):format(variant) );
    lib = root( ('%s/lib'):format(variant) );
    obj = root( ('%s/obj/cc_%s_x86_64'):format(variant, operating_system()) );
    include_directories = {
        root( 'src' );
    };
    library_directories = {
        root( ('%s/lib'):format(variant) );
    };
    assertions = variant ~= 'shipping';
    debug = variant ~= 'shipping';
};

cc:install( 'forge.cc' );

cc:all {
    'src/executable/all';
};

buildfile 'src/executable/executable.forge';
buildfile 'src/library/library.forge';
