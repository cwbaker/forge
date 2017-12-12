
SourcePrototype = TargetPrototype { "Source" };

function SourcePrototype.depend( source )
    for _, value in ipairs(source) do
        local source_file = file( value );
        assert( source_file, "Failed to find source file '%s' for scanning" % tostring(value) );
        scan( source_file, CcScanner );
    end
end

function SourcePrototype.build( source )
    if source:is_outdated() then
        cc( source );
    end
end

function SourcePrototype.clean( source )
    for dependency in source:get_dependencies() do
        if dependency:prototype() == nil then
            rm( dependency:path() );
        end
    end
end

function Source( definition )
    assert( type(definition) == "table" );

    local source = target( "", SourcePrototype, definition );
    local settings = build.current_settings( source.settings );
    source.settings = settings;

    if build.built_for_platform_and_variant(source) then
        local directory = Directory( obj_directory(source) );

        local pch = nil;
        -- if source.pch then
        --     presourced_header = PresourcedHeader( obj_directory(source)..obj_name(source.pch) );
        --     presourced_header.header = source.pch;
        --     presourced_header.unit = source;
        --     presourced_header:add_dependency( directory );
        --     source:add_dependency( presourced_header );
        --     source.presourced_header = presourced_header;
        -- end
        
        for _, value in ipairs(source) do
            local source_file = file( value );
            source_file:set_required_to_exist( true );
            source_file.unit = source;
            source_file.settings = settings;
            
            local object = file( "%s/%s" % {obj_directory(source_file), obj_name(value, source.architecture)} );
            object.source = value;
            source.object = object;
            object:add_dependency( source_file );
            object:add_dependency( directory );
            object:add_dependency( pch );      
            source:add_dependency( object );
        end
    end
    
    return source;
end
