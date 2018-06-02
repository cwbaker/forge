
local Dex = build:TargetPrototype( "android.Dex" );

function Dex.depend( build, target, dependencies )
    local jars = dependencies.jars;
    if jars then 
        java.add_jar_dependencies( target, dependencies.jars );
        dependencies.jars = nil;
    end
    return build.Target.depend( build, target, dependencies );
end

function Dex.build( build, target )
    local function add_jars( jars, other_jars )
        if other_jars then 
            for _, jar in ipairs(other_jars) do 
                table.insert( jars, jar );
            end
        end        
    end

    local jars = {};
    local settings = target.settings;
    local proguard = target:dependency( 1 );
    if proguard and target.settings.android.proguard_enabled then 
        local proguard_sh = ("%s/bin/proguard.sh"):format( target.settings.android.proguard_directory );
        build:system( proguard_sh, {
            'proguard.sh',
            ('-printmapping \"%s/%s.map\"'):format( settings.classes_directory(target), build:leaf(target) ),
            ('"@%s"'):format( proguard ) 
        } );
        table.insert( jars, ('\"%s/classes.jar\"'):format(settings.classes_directory(target)) );
    else
        table.insert( jars, settings.classes_directory(target) );
    end

    add_jars( jars, target.third_party_jars );
    add_jars( jars, settings.third_party_jars );

    for _, dependency in target:dependencies() do 
        local prototype = dependency:prototype();
        if prototype == build.Jar then 
            table.insert( jars, build:relative(dependency:filename()) );
        elseif prototype == build.Ivy then 
            for _, archive in dependency:implicit_dependencies() do 
                if build:extension(archive) == '.jar' then 
                    table.insert( jars, archive:filename() );
                else
                    table.insert( jars, ('%s/classes.jar'):format(archive:filename()) );
                end
            end
        end
    end

    local dx = build:native( ("%s/dx"):format(target.settings.android.build_tools_directory) );
    if build:operating_system() == "windows" then
        dx = ("%s.bat"):format( dx );
    end
    build:shell( {
        ('\"%s\"'):format( dx ),
        '--dex',
        '--verbose',
        ('--output=\"%s\"'):format( target ), 
        ('%s'):format( table.concat(jars, " ") )
    } );
end

android.Dex = Dex;
