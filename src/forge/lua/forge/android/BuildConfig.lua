
local BuildConfig = forge:TargetPrototype( 'BuildConfig' );

function BuildConfig.create( forge, settings, packages )
    local build_config = forge:Target( forge:anonymous(), BuildConfig );
    build_config.settings = settings;
    build_config.packages = packages;
    for index, package in ipairs(packages) do 
        local filename = forge:generated( ("%s/BuildConfig.java"):format(package:gsub("%.", "/")), nil, settings );
        build_config:set_filename( filename, index );
        build_config:add_ordering_dependency( forge:Directory(forge:branch(filename)) );
    end
    build_config:add_implicit_dependency( forge:current_buildfile() );
    return build_config;
end

function BuildConfig.build( forge, target )
    local HEADER = [[
/** Automatically generated file. DO NOT MODIFY */
package %s;

public final class BuildConfig {
]];

    local BOOLEAN_BODY = [[
    public final static boolean %s = %s;
]];

    local INT_BODY = [[
    public final static int %s = %s;
]];

    local FLOAT_BODY = [[
    public final static float %s = %sf;
]];

    local STRING_BODY = [[
    public final static String %s = "%s";
]];

    local FOOTER = [[
}
]];

    for index, package in ipairs(target.packages) do
        local filename = target:filename( index );
        local output_file = io.open( filename, "wb" );
        assert( output_file, ("Opening '%s' to write generated text failed"):format(filename) );
        output_file:write( HEADER:format(package) );
        if target.DEBUG == nil then 
            output_file:write( BOOLEAN_BODY:format("DEBUG", tostring(target.settings.debug)) );
        end
        for key, value in pairs(target) do 
            if type(value) == "boolean" then
                output_file:write( BOOLEAN_BODY:format(key, tostring(value)) );
            elseif type(value) == "number" then
                if math.floor(value) == value then 
                    output_file:write( INT_BODY:format(key, value) );
                else
                    output_file:write( FLOAT_BODY:format(key, value) );
                end
            elseif type(value) == "string" then
                output_file:write( STRING_BODY:format(key, value) );
            end
        end
        output_file:write( FOOTER:format(package) );
        output_file:close();
        output_file = nil;
    end
end

android.BuildConfig = BuildConfig;
