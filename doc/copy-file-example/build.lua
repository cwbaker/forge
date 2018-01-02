
require "build";

build:initialize();

build:all {
    build:Copy 'foo.out' {
        'foo.in'
    };    
};
