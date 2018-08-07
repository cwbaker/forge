
local Target = build.Target;

setmetatable( Target, {
    __call = function( _, build, identifier, target_prototype )
        local target = build:target( identifier, target_prototype );
        getmetatable( target ).__call = function( target, ... )
            local depend_function = target.depend;
            depend_function( target.build_, target, ... );
            return target;
        end;
        target.build_ = build;
        return target;
    end
} );

function Target.depend( build, target, dependencies )
    local settings = target.settings;
    if type(dependencies) == "string" then
        local source_file = build:SourceFile( dependencies, settings );
        target:add_dependency( source_file );
    elseif type(dependencies) == "table" then
        build:merge( target, dependencies );
        for _, value in ipairs(dependencies) do 
            local source_file = build:SourceFile( value, settings );
            target:add_dependency( source_file );
        end
    end
    return target;
end
