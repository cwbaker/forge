
local Java = build:TargetPrototype( "Java" );

function Java.create( build, settings )
    local java_ = build:Target( build:anonymous(), Java );
    java_.settings = settings;
    java_:set_filename( ("%s/Java.%s.timestamp"):format(settings.classes_directory(java_), java_:id()) );
    java_:add_ordering_dependency( build:Directory(settings.classes_directory(java_)) );
    java.add_jar_dependencies( java_, settings.jars );
    return java_;
end

function Java.depend( build, target, dependencies )
    build:pushd( dependencies.sourcepath or "." );
    for _, value in ipairs(dependencies) do
        local source = build:SourceFile( value, settings );
        target:add_dependency( source );
    end
    build:popd();

    local jars = dependencies.jars;
    if jars then 
        java.add_jar_dependencies( target, jars );
        dependencies.jars = nil;
    end

    build:merge( target, dependencies );
    return target
end

function Java.build( build, target )
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
            table.insert( source_files, build:relative(dependency:filename()) );
        elseif prototype == build.Jar then
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
            '-Xlint:unchecked',
            ('-d "%s"'):format(output),
            ('-sourcepath "%s"'):format(table.concat(sourcepaths, ':')),
            ('-classpath "%s"'):format(classpath),
            '-target 1.7',
            ('-bootclasspath "%s"'):format(table.concat(jars, ':')),
            '-encoding UTF-8',
            '-g',
            '-source 1.7',
            ('%s'):format(table.concat(source_files, ' '))
        };
        build:system( javac, command_line );

        local timestamp_file = io.open( target:filename(), "wb" );
        assertf( timestamp_file, "Opening '%s' to write generated text failed", target:filename() );
        timestamp_file:write( ("# Timestamp for '%s'"):format(target:path()) );
        timestamp_file:close();
        timestamp_file = nil;
    end
end

function Java.clean( build, target )
    build:rm( target:filename() );
    build:rmdir( settings.classes_directory(target) )
end

java.Java = Java;
