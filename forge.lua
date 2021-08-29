
-- Reference Lua build scripts from within the source tree rather than 
-- relative to the build executable so that they can be edited in place during
-- development and are versioned along with the code that they are building.
package.path = root('src/forge/lua/?.lua')..';'..root('src/forge/lua/?/init.lua');

variant = variant or 'debug';
architecture = architecture or 'native';

local cc = require 'forge.cc' {
    identifier = 'cc_${platform}_${architecture}';
    platform = operating_system();
    bin = root( ('%s/bin'):format(variant) );
    lib = root( ('%s/lib'):format(variant) );
    obj = root( ('%s/obj/cc_%s_x86_64'):format(variant, operating_system()) );
    include_directories = {
        root( 'src' );
        root( 'src/boost' );
        root( 'src/lua/src' );
        root( 'src/unittest-cpp' );
    };
    library_directories = {
        root( ('%s/lib'):format(variant) ),
    };
    defines = {
        ('BUILD_VARIANT_%s'):format( upper(variant) );
    };

    architecture = architecture;
    assertions = variant ~= 'shipping';
    debug = variant ~= 'shipping';
    debuggable = variant ~= 'shipping';
    exceptions = true;
    fast_floating_point = variant ~= 'debug';
    incremental_linking = variant == 'debug';
    link_time_code_generation = variant == 'shipping';
    minimal_rebuild = variant == 'debug';
    optimization = variant ~= 'debug';
    run_time_checks = variant == 'debug';
    runtime_library = variant == 'debug' and 'static_debug' or 'static_release';
    run_time_type_info = true;
    stack_size = 1048576;
    standard = 'c++11';
    string_pooling = variant == 'shipping';
    strip = false;
    warning_level = 3;
    warnings_as_errors = true;
};

local settings = cc.settings;

-- Bump the C++ standard to c++14 when building on Windows as that is the 
-- closest standard supported by Microsoft Visual C++.
if settings.platform == 'windows' then
    settings.standard = 'c++14';
end

buildfile 'src/assert/assert.forge';
buildfile 'src/boost/boost.forge';
buildfile 'src/cmdline/cmdline.forge';
buildfile 'src/error/error.forge';
buildfile 'src/forge/forge.forge';
buildfile 'src/lua/lua.forge';
buildfile 'src/luaxx/luaxx.forge';
buildfile 'src/process/process.forge';
buildfile 'src/unittest-cpp/unittest-cpp.forge';

cc:all {
    'src/forge/forge/all';
    'src/forge/forge_hooks/all';
    'src/forge/forge_test/all';
};

function install()
    prefix = prefix and root( prefix ) or home( 'forge' );
    local failures = default();
    if failures == 0 then 
        cc:cpdir( '${prefix}/bin', '${bin}' );
        cc:cpdir( '${prefix}/lua', 'src/forge/lua' );
    end
end
