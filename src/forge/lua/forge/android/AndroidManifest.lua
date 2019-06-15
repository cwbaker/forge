
local AndroidManifest = forge:FilePrototype( 'AndroidManifest' );

function AndroidManifest.build( forge, target )
    local manifests = {};
    for _, dependency in target:dependencies() do 
        if dependency:prototype() == forge.Ivy then 
            for _, archive in dependency:implicit_dependencies() do 
                if extension(archive) ~= '.jar' then
                    table.insert( manifests, ('%s/AndroidManifest.xml'):format(archive) );
                end
            end
        elseif dependency:id() == 'AndroidManifest.xml' then 
        	table.insert( manifests, dependency:filename() );
        elseif dependency:filename() ~= '' then
            table.insert( manifests, ('%s/AndroidManifest.xml'):format(dependency) );
        end
    end
	assertf( #manifests > 0, 'Missing main "AndroidManifest.xml" as first dependency of AndroidManifest "%s"', target:path() );

    local settings = forge.settings;
    local java = ('%s/bin/java'):format( settings.android.jdk_directory );
    local manifest_merger_jar = settings.android.manifest_merger;
    local args = {
        'java';
        ('-jar "%s"'):format( manifest_merger_jar );
        ('--log VERBOSE');
        ('--main "%s"'):format( manifests[1] );
        ('--out "%s"'):format( target );
    };

    if #manifests > 1 then 
        table.insert( args, ('--libs "%s"'):format(table.concat(manifests, ':', 2)) );
	end

    system( java, args );
end

return AndroidManifest;
