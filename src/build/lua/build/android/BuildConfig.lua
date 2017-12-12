
local BuildConfig = build.TargetPrototype( "android.BuildConfig" );

function BuildConfig.build( build_config )
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

    local output = build.interpolate( build_config_template, variables );
    local output_file = io.open( build_config:filename(), "wb" );
    assert( output_file, ("Opening '%s' to write generated text failed"):format(build_config:filename()) );
    output_file:write( output );
    output_file:close();
    output_file = nil;
end

android.BuildConfig = BuildConfig;
