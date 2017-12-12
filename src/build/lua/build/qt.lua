
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

QtMocPrototype = TargetPrototype { "QtMoc" };

function QtMocPrototype.static_depend( qtmoc )
    qtmoc:build();
end

function QtMocPrototype.build( qtmoc )
    if not exists(qtmoc:get_filename()) or qtmoc:is_outdated() then
        local moc = build.settings.qt.moc;
        print( qtmoc[1] );
        build.system( moc, [[moc %s -o %s]] % {qtmoc[1], relative(qtmoc:get_filename())} );
    end
end

function QtMocPrototype.clean( qtmoc )
    rm( qtmoc:get_filename() );
end

function QtMoc( definition )
    assert( definition and type(definition) == "table" );

    local cxx_files = {};
    for _, value in ipairs(definition) do
        local cxx_file = "moc_%s.cpp" % basename( value );
        table.insert( cxx_files, cxx_file );

        local qtmoc = target( cxx_file, QtMocPrototype, {value} );
        qtmoc:set_filename( qtmoc:path() );

        local header = file( value );
        header:set_required_to_exist( true );
        qtmoc:add_dependency( header );
    end

    return Cxx( cxx_files );
end
