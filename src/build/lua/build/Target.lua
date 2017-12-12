
local Target = build.Target;

setmetatable( Target, {
    __call = function( _, id, target_prototype, definition )
        local target_ = build.target( id, target_prototype, definition );
        getmetatable( target_ ).__call = function( target, ... )
            local depend_function = target.call or target.depend or Target.depend;
            depend_function( target, ... );
            return target;
        end;
        return target_;
    end
} );

function Target.depend( target, dependencies )
    local settings = build.current_settings();
    if type(dependencies) == "string" then
        local source_file = build.SourceFile( dependencies, settings );
        target:add_dependency( source_file );
    elseif type(dependencies) == "table" then
        build.merge( target, dependencies );
        for _, value in ipairs(dependencies) do 
            local source_file = build.SourceFile( value, settings );
            target:add_dependency( source_file );
        end
    end
    return target;
end

function Target.implicit_depend( target, dependencies )
    local settings = build.current_settings();
    if type(dependencies) == "string" then
        local source_file = build.SourceFile( dependencies, settings );
        target:add_implicit_dependency( source_file );
    elseif type(dependencies) == "table" then
        build.merge( target, dependencies );
        for _, value in ipairs(dependencies) do 
            local source_file = build.SourceFile( value, settings );
            target:add_implicit_dependency( source_file );
        end
    end
    return target;
end
