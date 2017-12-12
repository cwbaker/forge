
local TargetPrototype = build.TargetPrototype;

setmetatable( TargetPrototype, {
    __call = function( _, id, create_function )
        -- Split a '.' delimited string into a table hierarchy returning the last 
        -- level table and identifier.
        local function split_modules( module, qualified_id )
            local start = qualified_id:find( ".", 1, true );
            if start then
                local id = qualified_id:sub( 1, start - 1 );
                local submodule = module[id];
                if submodule == nil then 
                    submodule = {};
                    module[id] = submodule;
                end
                local remaining = qualified_id:sub( start + 1 );
                return split_modules( submodule, remaining );
            end
            return module, qualified_id;
        end

        local function create( target_prototype, ... )
            local identifier_or_sources = select( 1, ... );
            if type(identifier_or_sources) ~= "table" then   
                local settings = build.current_settings();
                local identifier = build.interpolate( identifier_or_sources, settings );
                local target = build.Target( identifier, target_prototype, select(2, ...) );
                if build.is_relative(identifier) then 
                    target:set_filename( build.object(identifier) );
                else
                    target:set_filename( identifier );
                end
                target:set_cleanable( true );
                target:add_ordering_dependency( build.Directory(build.branch(target)) );
                target.settings = settings;
                return target;
            end
        end

        local target_prototype = build.target_prototype( id );
        getmetatable( target_prototype ).__call = create_function or function( target_prototype, ... )
            local create_function = target_prototype.create;
            if create_function then 
                local settings = build.current_settings();
                return create_function( settings, ... );
            end
            return create( target_prototype, ... );
        end;

        local module, id = split_modules( build, id );
        module[id] = target_prototype;
        return target_prototype;
    end
} );
