
local DynamicLibrary = build.TargetPrototype( "DynamicLibrary" );

function DynamicLibrary.create( settings, id, architecture )
    architecture = architecture or settings.default_architecture
    local dynamic_library = build.Target( ("%s_%s"):format(id, architecture), DynamicLibrary );
    dynamic_library.settings = settings;
    dynamic_library.architecture = architecture;
    dynamic_library:set_filename( ("%s/%s"):format(settings.bin, dll_name(id, architecture)) );    
    dynamic_library:add_ordering_dependency( build.Directory(build.branch(dynamic_library:filename())) );
    return dynamic_library;
end

function DynamicLibrary.call( dynamic_library, definition )
    build.merge( dynamic_library, definition );
    build.add_library_dependencies( dynamic_library, definition.libraries );
    for _, dependency in ipairs(definition) do 
        dependency.module = dynamic_library;
        dynamic_library:add_dependency( dependency );
    end
end

function DynamicLibrary.build( dynamic_library )
    if dynamic_library:outdated() then
        build_executable( dynamic_library );
    end
end

function DynamicLibrary.clean( dynamic_library )
    clean_executable( dynamic_library );
end
