
local Target = forge.Target;

setmetatable( Target, {
    __call = function( _, forge, identifier, target_prototype )
        local target = forge:target( identifier, target_prototype );
        getmetatable( target ).__call = function( target, ... )
            local depend_function = target.depend;
            depend_function( target.forge_, target, ... );
            return target;
        end;
        target.forge_ = forge;
        return target;
    end
} );

function Target.depend( forge, target, dependencies )
    local settings = target.settings;
    if type(dependencies) == "string" then
        local source_file = forge:SourceFile( dependencies, settings );
        target:add_dependency( source_file );
    elseif type(dependencies) == "table" then
        forge:merge( target, dependencies );
        for _, value in ipairs(dependencies) do 
            local source_file = forge:SourceFile( value, settings );
            target:add_dependency( source_file );
        end
    end
    return target;
end
