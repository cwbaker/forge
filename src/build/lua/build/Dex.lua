
DexPrototype = TargetPrototype { "Dex" };

function DexPrototype.build( jar )
    if jar:is_outdated() then
        print( leaf(jar:get_filename()) );

        local jars = {};
        if jar.settings.android.proguard_enabled then 
            local proguard = "%s/tools/proguard/bin/proguard.sh" % jar.settings.android.sdk_directory;
            build.system( proguard, [[proguard.sh -printmapping \"%s/%s.map\" @proguard.cfg]] % {obj_directory(jar), leaf(jar:get_filename())} );
            table.insert( jars, [[\"%s/classes.jar\"]] % obj_directory(jar) );
        else
            table.insert( jars, [[\"%s/classes\"]] % obj_directory(jar) );
        end
        for _, library in ipairs(jar.settings.libraries) do 
            table.insert( jars, library );
        end

        local dx = native( "%s/dx" % jar.settings.android.build_tools_directory );
        if operating_system() == "windows" then
            dx = "%s.bat" % dx;
        end
        local source = table.concat( jars, " " );
        build.shell( [[\"%s\" --dex --verbose --output=\"%s\" %s]] % {dx, jar:get_filename(), table.concat(jars, " ")} );
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
                build.add_module_dependencies( jar, "%s/%s.dex" % {settings.bin, id}, "", settings );
                table.insert( jars, jar );
            end
            build.pop_settings();
            return jars;
        end);
    end
end
