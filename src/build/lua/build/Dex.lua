
DexPrototype = TargetPrototype { "Dex" };

function DexPrototype.build( jar )
    if jar:is_outdated() then
        print( leaf(jar:get_filename()) );
        local dx = native( "%s/platform-tools/dx.bat" % jar.settings.android.sdk_directory );
        build.shell( [["%s" --dex --verbose --output="%s" "%s/classes"]] % {dx, jar:get_filename(), obj_directory(jar)} );
    end    
end

function DexPrototype.clean( jar )
    rm( jar:get_filename() );
end

function Dex( id )
    return function( dependencies )
        local settings = build.current_settings();
        local jar = target( "", DexPrototype, dependencies );
        build.add_module_dependencies( jar, "%s/%s.dex" % {settings.bin, id}, settings );
        return { jar };
    end
end
