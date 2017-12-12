
local Dex = build.TargetPrototype( "android.Dex" );

function Dex.create( _, id, settings )
    local settings = settings or build.current_settings();
    local dex = build.Target( "", Dex, definition );
    dex:set_filename( ("%s/%s.dex"):format(settings.bin, id) );
    dex.settings = settings;
    build.add_jar_dependencies( dex, settings.jars );
    return dex;
end

function Dex.call( dex, definition )
    build.add_jar_dependencies( dex, definition.jars );
    for _, dependency in ipairs(definition) do 
        dex:add_dependency( dependency );
        dependency.module = dex;
    end
end

function Dex.build( dex )
    if dex:is_outdated() then
        print( leaf(dex:get_filename()) );

        local jars = {};
        if dex.settings.android.proguard_enabled then 
            local proguard = ("%s/tools/proguard/bin/proguard.sh"):format( dex.settings.android.sdk_directory );
            build.system( proguard, ('proguard.sh -printmapping \"%s/%s.map\" @proguard.cfg'):format(obj_directory(dex), leaf(dex:get_filename())) );
            table.insert( jars, ('\"%s/classes.dex\"'):format(build.classes_directory(dex)) );
        else
            table.insert( jars, build.classes_directory(dex) );
        end
        for dependency in dex:get_dependencies() do 
            if dependency:prototype() == Jar then 
                table.insert( jars, relative(dependency:get_filename()) );
            end
        end
        if dex.third_party_jars then 
            for _, jar in ipairs(dex.third_party_jars) do 
                table.insert( jars, jar );
            end
        end
        if dex.settings.third_party_jars then
            for _, jar in ipairs(dex.settings.third_party_jars) do 
                table.insert( jars, jar );
            end
        end

        local dx = native( ("%s/dx"):format(dex.settings.android.build_tools_directory) );
        if operating_system() == "windows" then
            dx = ("%s.bat"):format( dx );
        end
        build.shell( ('\"%s\" --dex --verbose --output=\"%s\" %s'):format(dx, dex:get_filename(), table.concat(jars, " ")) );
    end    
end

function Dex.clean( dex )
    rm( dex:get_filename() );
end

android.Dex = Dex;
