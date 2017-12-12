
local Dex = build.TargetPrototype( "android.Dex" );

function Dex.create( settings, id )
    local dex = build.Target( "", Dex, definition );
    dex:set_filename( ("%s/%s.dex"):format(settings.bin, id) );
    dex.settings = settings;
    dex:add_ordering_dependency( build.Directory(dex:branch()) );
    dex:add_ordering_dependency( build.Directory(build.classes_directory(dex)) );
    build.add_jar_dependencies( dex, settings.jars );
    return dex;
end

function Dex.call( dex, definition )
    build.add_jar_dependencies( dex, definition.jars );
    for _, dependency in ipairs(definition) do 
        dex:add_dependency( dependency );
        dependency.module = dex;
    end
    local proguard = definition.proguard;
    if proguard then 
        dex:add_dependency( proguard );
        dex.proguard = proguard;
    end
end

function Dex.build( dex )
    if dex:outdated() then
        print( build.leaf(dex:filename()) );

        local jars = {};
        if dex.proguard and dex.settings.android.proguard_enabled then 
            local proguard = dex.proguard; 
            local proguard_sh = ("%s/tools/proguard/bin/proguard.sh"):format( dex.settings.android.sdk_directory );
            build.system( proguard_sh, ('proguard.sh -printmapping \"%s/%s.map\" "@%s"'):format(build.classes_directory(dex), build.leaf(dex:filename()), proguard:filename()) );
            table.insert( jars, ('\"%s/classes.jar\"'):format(build.classes_directory(dex)) );
        else
            table.insert( jars, build.classes_directory(dex) );
        end
        for _, dependency in dex:dependencies() do 
            if dependency:prototype() == build.Jar then 
                table.insert( jars, build.relative(dependency:filename()) );
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

        local dx = build.native( ("%s/dx"):format(dex.settings.android.build_tools_directory) );
        if build.operating_system() == "windows" then
            dx = ("%s.bat"):format( dx );
        end
        build.shell( ('\"%s\" --dex --verbose --output=\"%s\" %s'):format(dx, dex:filename(), table.concat(jars, " ")) );
    end    
end

function Dex.clean( dex )
    build.rm( dex:filename() );
end

android.Dex = Dex;
