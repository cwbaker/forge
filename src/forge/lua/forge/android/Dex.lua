
local Dex = forge:FilePrototype( 'Dex' );

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
    for _, dependency in target:dependencies() do 
        local prototype = dependency:prototype();
        if prototype == forge.Jar then 
            table.insert( jars, relative(dependency:filename()) );
        elseif prototype == forge.Java then 
            local classes = dependency:ordering_dependency():filename();
            local proguard = target:dependency( 1 );
            if proguard and proguard:id() == 'proguard.cfg' and settings.android.proguard_enabled then 
                local proguard_sh = ('%s/bin/proguard.sh'):format( settings.android.proguard_directory );
                forge:system( proguard_sh, {
                    'proguard.sh',
                    ('-printmapping \"%s/%s.map\"'):format( classes, leaf(target) ),
                    ('"@%s"'):format( proguard ) 
                } );
                table.insert( jars, ('\"%s/classes.jar\"'):format(classes) );
            else
                table.insert( jars, classes );
            end
        elseif prototype == forge.Ivy then 
            for _, archive in dependency:implicit_dependencies() do 
                if extension(archive) == '.jar' then 
                    table.insert( jars, archive:filename() );
                else
                    table.insert( jars, ('%s/classes.jar'):format(archive:filename()) );
                end
            end
        end
    end

    add_jars( jars, target.jars );
    add_jars( jars, settings.jars );

    local dx = native( ('%s/dx'):format(settings.android.build_tools_directory) );
    if operating_system() == 'windows' then
        dx = ('%s.bat'):format( dx );
    end
    forge:shell( {
        ('"%s"'):format( dx ),
        '--dex',
        '--verbose',
        ('--output="%s"'):format( target ), 
        ('%s'):format( table.concat(jars, ' ') )
    } );
end

return Dex;
