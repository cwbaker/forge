
local Target = forge.Target;

function Target.create( forge, identifier, target_prototype )
    local target = forge:add_target( identifier, target_prototype );
    target.settings = forge.settings;
    return target;
end

function Target.depend( forge, target, dependencies )
    assert( type(dependencies) == 'table', 'Target.depend() parameter not a table as expected' );

    local function typename( value )
        if type(value) == 'table' then 
            local metatable = getmetatable( value );
            return metatable and metatable.__name;
        end
    end

    local function visit( value )
        if type(value) ~= 'table' or typename(value) == 'forge.Target' then
            local source_file = forge:SourceFile( value );
            target:add_dependency( source_file );
            return true;
        end
    end

    local function walk( dependencies, visit )
        for _, value in ipairs(dependencies) do 
            if not visit(value) then 
                walk( value, visit );
            end
        end
    end

    forge:merge( target, dependencies );

    walk( dependencies, function (value) 
        if type(value) ~= 'table' or typename(value) == 'forge.Target' then
            local source_file = forge:SourceFile( value );
            target:add_dependency( source_file );
            return true;
        end
    end );
end

return Target;
