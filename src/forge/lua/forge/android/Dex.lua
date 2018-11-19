
local Dex = forge:FilePrototype( 'Dex' );

function Dex.depend( forge, target, dependencies )
    local jars = dependencies.jars;
    if jars then 
        java.add_jar_dependencies( target, dependencies.jars );
        dependencies.jars = nil;
    end
    return forge.Target.depend( forge, target, dependencies );
end

function Dex.build( forge, target )
    local function add_jars( jars, other_jars )
        if other_jars then 
            for _, jar in ipairs(other_jars) do 
                table.insert( jars, jar );
            end
        end        
    end

    local jars = {};
    local settings = forge.settings;
    local proguard = target:dependency( 1 );
    if proguard and settings.android.proguard_enabled then 
        local proguard_sh = ("%s/bin/proguard.sh"):format( settings.android.proguard_directory );
        forge:system( proguard_sh, {
            'proguard.sh',
            ('-printmapping \"%s/%s.map\"'):format( settings.classes_directory(target), forge:leaf(target) ),
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
        if prototype == forge.Jar then 
            table.insert( jars, forge:relative(dependency:filename()) );
        elseif prototype == forge.Ivy then 
            for _, archive in dependency:implicit_dependencies() do 
                if forge:extension(archive) == '.jar' then 
                    table.insert( jars, archive:filename() );
                else
                    table.insert( jars, ('%s/classes.jar'):format(archive:filename()) );
                end
            end
        end
    end

    local dx = forge:native( ("%s/dx"):format(settings.android.build_tools_directory) );
    if forge:operating_system() == "windows" then
        dx = ("%s.bat"):format( dx );
    end
    forge:shell( {
        ('\"%s\"'):format( dx ),
        '--dex',
        '--verbose',
        ('--output=\"%s\"'):format( target ), 
        ('%s'):format( table.concat(jars, " ") )
    } );
end
