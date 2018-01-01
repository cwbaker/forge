
package.path = build:default_package_path();

require "build";

build:initialize {
    cache = 'copy-file-example.cache';
};

build:all {
    build:Copy 'foo.out' {
        'foo.in'
    };    
};
