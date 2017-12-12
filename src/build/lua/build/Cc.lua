
CcPrototype = TargetPrototype { "Cc" };

function CcPrototype.depend( source )
    for _, value in ipairs(source) do
        local source_file = file( value );
        assert( source_file, "Failed to find source file '%s' for scanning" % tostring(value) );
        scan( source_file, CcScanner );
    end
end

function CcPrototype.build( source )
    if source:is_outdated() then
        cc( source );
    end
end

function CcPrototype.clean( source )
    for dependency in source:get_dependencies() do
        if dependency:prototype() == nil then
            rm( dependency:path() );
        end
    end
end

function Cc( definition )
    definition.language = "c";
    build.begin_target();
    return build.end_target( build.add_unit_dependencies(definition, CcPrototype) );
end

function Cxx( definition )
    definition.language = "c++";
    build.begin_target();
    return build.end_target( build.add_unit_dependencies(definition, CcPrototype) );
end

function ObjC( definition )
    definition.language = "objective-c";
    build.begin_target();
    return build.end_target( build.add_unit_dependencies(definition, CcPrototype) );
end

function ObjCxx( definition )
    definition.language = "objective-c++";
    build.begin_target();
    return build.end_target( build.add_unit_dependencies(definition, CcPrototype) );
end
