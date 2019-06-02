
-- Reference Lua build scripts from within the source tree rather than 
-- relative to the build executable so that they can be edited in place during
-- development and are versioned along with the code that they are building.
package.path = forge:root('src/forge/lua/?.lua')..';'..forge:root('src/forge/lua/?/init.lua');

variant = forge:lower( variant or 'debug' );

local forge = require 'forge.cc' {
    identifier = 'cc_${platform}_${architecture}';
    platform = forge:operating_system();
    bin = forge:root( ('%s/bin'):format(variant) );
    lib = forge:root( ('%s/lib'):format(variant) );
    obj = forge:root( ('%s/obj/cc_%s_x86_64'):format(variant, forge:operating_system()) );
    include_directories = {
        forge:root( 'src' );
        forge:root( 'src/boost' );
        forge:root( 'src/lua/src' );
        forge:root( 'src/unittest-cpp' );
    };
    library_directories = {
        forge:root( ('%s/lib'):format(variant) ),
    };
    defines = {
        ('BUILD_VARIANT_%s'):format( forge:upper(variant) );
    };
    xcode = {
        xcodeproj = forge:root( 'forge.xcodeproj' );
    };

    architecture = 'x86_64';
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

-- Bump the C++ standard to c++14 when building on Windows as that is the 
-- closest standard supported by Microsoft Visual C++.
local settings = forge.settings;
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

forge:all {
    'src/forge/forge/all';
    'src/forge/forge_hooks/all';
    'src/forge/forge_test/all';
};

function install()
    prefix = prefix and forge:root( prefix ) or forge:home( 'forge' );
    local failures = default();
    if failures == 0 then 
        forge:cpdir( '${prefix}/bin', '${bin}' );
        forge:cpdir( '${prefix}/lua', 'src/forge/lua' );
    end
end
