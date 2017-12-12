
DexPrototype = TargetPrototype { "Dex" };

function DexPrototype.static_depend( jar )
    build.add_jar_dependencies( jar );
end

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
        if jar.jars then 
            for _, jar in ipairs(jar.jars) do 
                table.insert( jars, jar:get_filename() );
            end
        end
        if jar.third_party_jars then 
            for _, jar in ipairs(jar.third_party_jars) do 
                table.insert( jars, jar );
            end
        end
        if jar.settings.third_party_jars then
            for _, jar in ipairs(jar.settings.third_party_jars) do 
                table.insert( jars, jar );
            end
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
                build.push_settings {
                    classes = "%s/%s_%s/%s/classes" % { settings.obj, platform, variant, relative(jar:get_working_directory():path(), root()) };
                };
                build.add_module_dependencies( jar, "%s/%s.dex" % {settings.bin, id}, "", settings );
                build.pop_settings();
                table.insert( jars, jar );
            end
            build.pop_settings();
            return jars;
        end);
    end
end
