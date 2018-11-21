
local Java = forge:TargetPrototype( 'Java' );

function Java.create( forge, identifier )
    local settings = forge.settings;
    local target = forge:Target( forge:anonymous(), Java );
    target:set_filename( ("%s/Java.%s.timestamp"):format(settings.classes_directory(target), target:id()) );
    target:add_ordering_dependency( forge:Directory(settings.classes_directory(target)) );
    return target;
end

function Java.depend( forge, target, dependencies )
    local settings = forge.settings;
    forge:pushd( dependencies.sourcepath or "." );
    for _, value in ipairs(dependencies) do
        local source = forge:SourceFile( value, settings );
        target:add_dependency( source );
    end
    forge:popd();

    local jars = dependencies.jars;
    if jars then 
        dependencies.jars = nil;
    end

    forge:merge( target, dependencies );
    return target
end

function Java.build( forge, target )
    local function add_jars( jars, other_jars )
        if other_jars then 
            for _, jar in ipairs(other_jars) do 
                table.insert( jars, jar );
            end
        end        
    end

    local settings = target.settings;
    local jars = {};
    add_jars( jars, target.third_party_jars );
    add_jars( jars, target.system_jars );
    add_jars( jars, target.settings.third_party_jars );
    add_jars( jars, target.settings.system_jars );

    local source_files = {};
    for _, dependency in target:dependencies() do 
        local prototype = dependency:prototype();
        if prototype == nil then
            table.insert( source_files, forge:relative(dependency:filename()) );
        elseif prototype == forge.Jar then
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

    if #source_files > 0 then
        local javac = ("%s/bin/javac"):format( settings.java.jdk_directory );
        local output = settings.classes_directory( target );
        local classpath = output;

        local sourcepaths = {};
        if target.sourcepaths then
            for _, path in ipairs(target.sourcepaths) do 
                table.insert( sourcepaths, path );
            end
        end
        if settings.sourcepaths then 
            for _, path in ipairs(target.settings.sourcepaths) do 
                table.insert( sourcepaths, path );
            end
        end
        table.insert( sourcepaths, target.sourcepath or "." );
        table.insert( sourcepaths, settings.gen_directory(target) );
        
        local command_line = {
            'javac',
            '-source 1.7',
            '-target 1.7',
            '-g',
            '-Xlint:unchecked',
            '-encoding UTF-8',
            ('-d "%s"'):format(output),
            ('-sourcepath "%s"'):format(table.concat(sourcepaths, ':')),
            ('-classpath "%s"'):format(classpath),
            ('-bootclasspath "%s"'):format(table.concat(jars, ':')),
            ('%s'):format(table.concat(source_files, ' '))
        };
        forge:system( javac, command_line );

        local timestamp_file = io.open( target:filename(), "wb" );
        assertf( timestamp_file, "Opening '%s' to write generated text failed", target:filename() );
        timestamp_file:write( ("# Timestamp for '%s'"):format(target:path()) );
        timestamp_file:close();
        timestamp_file = nil;
    end
end

function Java.clean( forge, target )
    forge:rm( target:filename() );
    forge:rmdir( settings.classes_directory(target) )
end
