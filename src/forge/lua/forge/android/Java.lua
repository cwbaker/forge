
local Java = forge:JavaStylePrototype( 'Java' );

local function append( destination, source )
    if source then 
        for _, value in ipairs(source) do 
            table.insert( destination, value );
        end
    end
end

function Java.build( forge, target )
    local settings = forge.settings;

    local source_files = {};
    local classpaths = {};

    for _, dependency in target:dependencies() do 
        local prototype = dependency:prototype();
        if prototype == forge.Jar then
            table.insert( classpaths, relative(dependency:filename()) );
        elseif prototype == forge.Ivy then 
            for _, archive in dependency:implicit_dependencies() do 
                if extension(archive) == '.jar' then 
                    table.insert( classpaths, archive:filename() );
                else
                    table.insert( classpaths, ('%s/classes.jar'):format(archive:filename()) );
                end
            end
        else
            for _, filename in dependency:filenames() do 
                if filename ~= '' then
                    table.insert( source_files, relative(filename) );
                end
            end
        end
    end

    if #source_files > 0 then
        local javac = ('%s/bin/javac'):format( forge.settings.android.jdk_directory );
        local output_directory = target:ordering_dependency():filename();

        append( classpaths, target.jars );
        append( classpaths, settings.jars );
        table.insert( classpaths, output_directory );

        local sourcepaths = {};
        append( sourcepaths, target.sourcepaths );
        append( sourcepaths, settings.sourcepaths );
        table.insert( sourcepaths, target.sourcepath or '.' );

        local bootclasspaths = {};
        append( bootclasspaths, target.bootclasspaths );
        append( bootclasspaths, settings.bootclasspaths );
        
        local command_line = {
            'javac',
            '-source 1.7',
            '-target 1.7',
            '-Xlint:unchecked',
            '-encoding UTF-8',
            '-g',
            ('-d "%s"'):format(output_directory),
            ('-sourcepath "%s"'):format(table.concat(sourcepaths, ':')),
            ('-classpath "%s"'):format(table.concat(classpaths, ':')),
            ('-bootclasspath "%s"'):format(table.concat(bootclasspaths, ':')),
            ('%s'):format(table.concat(source_files, ' '))
        };
        system( javac, command_line, nil, forge:filenames_filter(target) );
    end
end

function Java.clean( forge, target )
    rm( target );
    rmdir( target:ordering_dependency() );
    target:set_built( false );
end

return Java;
