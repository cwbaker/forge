
local TargetPrototype = forge.TargetPrototype;

setmetatable( TargetPrototype, {
    __call = function( _, forge, identifier )
        -- Split a '.' delimited string into a table hierarchy returning the last 
        -- level table and identifier.
        local function split_modules( module, qualified_id )
            local start = qualified_id:find( ".", 1, true );
            if start then
                local identifier = qualified_id:sub( 1, start - 1 );
                local submodule = module[identifier];
                if submodule == nil then 
                    submodule = {};
                    module[identifier] = submodule;
                end
                local remaining = qualified_id:sub( start + 1 );
                return split_modules( submodule, remaining );
            end
            return module, qualified_id;
        end

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

        local module, identifier = split_modules( forge, identifier );
        module[identifier] = target_prototype;
        forge[identifier] = target_prototype;
        return target_prototype;
    end
} );
