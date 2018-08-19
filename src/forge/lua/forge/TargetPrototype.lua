
local TargetPrototype = forge.TargetPrototype;

setmetatable( TargetPrototype, {
    __call = function( _, forge, identifier )
        local function create( forge, target_prototype, identifier )
            local settings = forge:current_settings();
            local target = forge:File( identifier, target_prototype, settings );
            target:add_ordering_dependency( forge:Directory(forge:branch(target)) );
            target.settings = settings;
            return target;
        end

        local target_prototype = forge:target_prototype( identifier );
        getmetatable( target_prototype ).__call = function( target_prototype, forge, ... )
            local target;
            local create_function = target_prototype.create;
            if create_function then 
                local settings = forge:current_settings();
                target = create_function( forge, settings, ... );
            else
                target = create( forge, target_prototype, ... );
            end
            local annotate_function = target.annotate;
            if annotate_function then 
                annotate_function( forge, target, ... );
            end
            return target;
        end;

        forge[identifier] = target_prototype;
        return target_prototype;
    end
} );
