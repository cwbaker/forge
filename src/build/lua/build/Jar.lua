
JarPrototype = TargetPrototype { "Jar" };

function JarPrototype.build( jar )
    if jar:is_outdated() then
        print( leaf(jar:get_filename()) );
        local jar_ = native( "%s/bin/jar" % jar.settings.android.jdk_directory );
        pushd( "%s/classes" % obj_directory(jar) );
        build.system( jar_, [[jar cvf "%s" "."]] % {jar:get_filename()} );
        popd();
    end    
end

function JarPrototype.clean( jar )
    rm( jar:get_filename() );
end

function Jar( id )
    build.begin_target();
    return function( dependencies )
        return build.end_target( function()            
            local jars = {};
            local settings = build.push_settings( dependencies.settings );
            if build.built_for_platform_and_variant(settings) then
                local jar = target( "%s.jar" % id, JarPrototype, dependencies );
                build.push_settings {
                    classes = "%s/%s_%s/%s/classes" % { settings.obj, platform, variant, relative(jar:get_working_directory():path(), root()) };
                };
                build.add_module_dependencies( jar, "%s/%s.jar" % {settings.lib, id}, "", settings );
                build.pop_settings();
                table.insert( jars, jar );
            end
            build.pop_settings();
            return jars;
        end);
    end
end
