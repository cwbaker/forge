
local TargetPrototype = forge.TargetPrototype;

setmetatable( TargetPrototype, {
    __call = function( _, forge, identifier )
        local target_prototype = forge:target_prototype( identifier );
        getmetatable( target_prototype ).__call = function( target_prototype, forge, identifier, ... )
            local target;
            local create_function = target_prototype.create;
            if create_function then 
                local settings = forge:current_settings();
                target = create_function( forge, settings, identifier, ... );
            else
                target = forge:File( identifier, target_prototype );
            end
            return target;
        end;
        forge[identifier] = target_prototype;
        return target_prototype;
    end
} );