
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
    build.begin_target();
    return function( dependencies )
        return build.end_target( function()            
            local jars = {};
            local settings = build.push_settings( dependencies.settings );
            if build.built_for_platform_and_variant(settings) then
                local jar = target( "", DexPrototype, dependencies );
                build.add_module_dependencies( jar, "%s/%s.dex" % {settings.bin, id} );
                table.insert( jars, jar );
            end
            build.pop_settings();
            return jars;
        end);
    end
end
