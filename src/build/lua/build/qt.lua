
qt = {};

function qt.configure( settings )
    local local_settings = build.local_settings;
    if not local_settings.qt then
        local_settings.updated = true;
        local_settings.qt = {
            qt_directory = "C:/Qt/4.7.2";
            version = "4.7.2";
        };
        local_settings.qt.moc = "%s/bin/moc.exe" % { local_settings.qt.qt_directory };
    end
end

function qt.initialize( settings )
    qt.configure( settings );
end

QtMocPrototype = TargetPrototype { "QtMoc", BIND_GENERATED_FILE };

function QtMocPrototype.load( target )
    target:set_filename( target:path() );
    local header = file( target[1] );
    header:set_required_to_exist( true );
    target:add_dependency( header );
end

function QtMocPrototype.build( target )
    if target:is_outdated() then
        local moc = build.settings.qt.moc;
        print( target[1] );
        build.system( moc, [[moc %s -o %s]] % {target[1], target:id()} );
    end
end

function QtMocPrototype.clean( target )
    rm( target:id() );
end

function QtMoc( moc )
    assert( moc and type(moc) == "table" );
    local source = Source();
    for _, value in ipairs(moc) do
        local output = target( cxx_name("moc_%s" % value), QtMocPrototype, {value} );
        table.insert( source, output );        
    end
    return source;
end
