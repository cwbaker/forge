
local BuildConfig = build.TargetPrototype( "android.BuildConfig" );

function BuildConfig.create( settings, package )
    local working_directory = build.working_directory();
    local gen_directory = ("%s/%s"):format( settings.gen, build.relative(working_directory:path(), build.root()) );
    local build_config = build.File( ("%s/%s/BuildConfig.java"):format(gen_directory, string.gsub(package, "%.", "/")), BuildConfig );
    build_config.settings = settings;
    build_config.package = package;
    build_config:add_ordering_dependency( build.Directory(build_config:directory()) );
    return build_config;
end

function BuildConfig.build( build_config )
    if build_config:outdated() then
        local build_config_template = [[
/** Automatically generated file. DO NOT MODIFY */
package ${package};

public final class BuildConfig {
    public final static boolean DEBUG = ${debug};
}
]];
        local variables = {
            package = build_config.package;
            debug = tostring( build_config.settings.debug );
        };

        print( build.leaf(build_config:filename()) );
        local output = build.interpolate( build_config_template, variables );
        local output_file = io.open( build_config:filename(), "wb" );
        assert( output_file, ("Opening '%s' to write generated text failed"):format(build_config:filename()) );
        output_file:write( output );
        output_file:close();
        output_file = nil;
    end
end

function BuildConfig.clean( build_config )
    build.rm( build_config:filename() );
end

android.BuildConfig = BuildConfig;
