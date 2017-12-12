
AidlPrototype = TargetPrototype { "Aidl" };

function AidlPrototype.static_depend( aidl )
    aidl:build();
end

function AidlPrototype.build( aidl )
    if not exists(aidl:get_filename()) or aidl:is_outdated() then
        local settings = aidl.settings;
        local aidl_ = "%s/aidl" % settings.android.build_tools_directory;
        local output = absolute( "." );
        local platform = "%s/platforms/%s/framework.aidl" % { settings.android.sdk_directory, settings.android.sdk_platform };
        local source = aidl.source;
        build.system( aidl_, [[aidl -o%s -p%s %s]] % {output, platform, source} );
    end
end

function AidlPrototype.clean( aidl )
    rm( aidl:path() );
end

function Aidl( filename )
    build.begin_target();
    return function( source )
        return build.end_target( function()
            local source_file = file( source );
            source_file:set_required_to_exist( true );

            local aidl = file( filename, AidlPrototype );
            aidl.settings = settings;
            aidl.source = source;
            aidl:add_dependency( source_file );
            aidl:add_dependency( Directory(branch(aidl:get_filename())) );
            return aidl;
        end );
    end
end
