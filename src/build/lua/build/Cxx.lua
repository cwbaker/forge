
CxxPrototype = TargetPrototype { "Cxx" };

function CxxPrototype.depend( source )
    for _, value in ipairs(source) do
        local source_file = file( value );
        assert( source_file, "Failed to find source file '%s' for scanning" % tostring(value) );
        scan( source_file, CcScanner );
    end
end

function CxxPrototype.build( source )
    if source:is_outdated() then
        cc( source );
    end
end

function CxxPrototype.clean( source )
    for dependency in source:get_dependencies() do
        if dependency:prototype() == nil then
            rm( dependency:path() );
        end
    end
end
function Cxx( definition )
    build.begin_target();
    return build.end_target( function( architecture )
        local cxx;
        local settings = build.push_settings( definition.settings );
        if build.built_for_platform_and_variant(settings) then
            cxx = target( "", CxxPrototype, build.copy(definition) );
            cxx.settings = settings;
            cxx.architecture = architecture;

            for _, value in ipairs(cxx) do
                local source_file = file( value );
                source_file:set_required_to_exist( true );
                source_file.unit = cxx;
                source_file.settings = settings;

                local object = file( "%s/%s/%s" % {obj_directory(source_file), architecture, obj_name(value)} );
                object.source = value;
                object:add_dependency( source_file );
                object:add_dependency( Directory("%s/%s" % {obj_directory(source_file), architecture}) );
                cxx:add_dependency( object );
            end
        end
        build.pop_settings();        
        return cxx;
    end );
end
