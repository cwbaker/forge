
local BuildConfig = forge:JavaStylePrototype( 'BuildConfig' );

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

    local output_directory = target:ordering_dependency():filename();
    for index, package in ipairs(target.packages) do
        local filename = forge:interpolate( ('%s/%s/BuildConfig.java'):format(output_directory, package:gsub('%.', '/')) );
        target:add_filename( filename );
        mkdir( branch(filename) );
        local output_file = io.open( filename, 'wb' );
        assert( output_file, ('Opening "%s" to write generated text failed'):format(filename) );
        output_file:write( HEADER:format(package) );
        if target.DEBUG == nil then 
            output_file:write( BOOLEAN_BODY:format('DEBUG', tostring(forge.settings.debug or false)) );
        end
        for key, value in pairs(target) do 
            if type(value) == 'boolean' then
                output_file:write( BOOLEAN_BODY:format(key, tostring(value)) );
            elseif type(value) == 'number' then
                if math.floor(value) == value then 
                    output_file:write( INT_BODY:format(key, value) );
                else
                    output_file:write( FLOAT_BODY:format(key, value) );
                end
            elseif type(value) == 'string' then
                output_file:write( STRING_BODY:format(key, value) );
            end
        end
        output_file:write( FOOTER:format(package) );
        output_file:close();
        output_file = nil;
    end
end

return BuildConfig;
