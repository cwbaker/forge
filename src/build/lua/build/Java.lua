
JavaPrototype = TargetPrototype { "Java" };

function JavaPrototype.static_depend( java )
    build.add_jar_dependencies( java );
end

function JavaPrototype.build( java )
    if java:is_outdated() then
        local source_files = {};
        for dependency in java:get_dependencies() do 
            if dependency:is_outdated() then
                if dependency:prototype() == nil then
                    table.insert( source_files, dependency.source );
                end
            end    
        end

        if #source_files > 0 then
            local settings = java.settings;
            local javac = "%s/bin/javac" % settings.android.jdk_directory;
            local output = settings.classes;
            local classpath = output;

            local sourcepath = {
                "."
            };
            if java.sourcepath then
                for _, path in ipairs(java.sourcepath) do 
                    table.insert( sourcepath, path );
                end
            end
            if java.settings.sourcepath then 
                for _, path in ipairs(java.settings.sourcepath) do 
                    table.insert( sourcepath, path );
                end
            end

            local jars = {
                "%s/platforms/%s/android.jar" % { settings.android.sdk_directory, settings.android.sdk_platform };
            };            
            if java.jars then 
                for _, jar in ipairs(java.jars) do 
                    table.insert( jars, jar:get_filename() );
                end
            end
            if java.third_party_jars then 
                for _, jar in ipairs(java.third_party_jars) do 
                    table.insert( jars, jar );
                end
            end
            if settings.jars then 
                for _, jar in ipairs(settings.jars) do 
                    table.insert( jars, jar:get_filename() );
                end
            end
            if settings.third_party_jars then 
                for _, jar in ipairs(settings.third_party_jars) do 
                    table.insert( jars, jar );
                end
            end

            build.system( javac, [[javac -Xlint:unchecked -d "%s" -sourcepath "%s" -classpath "%s" -target 1.5 -bootclasspath "%s" -encoding UTF-8 -g -source 1.5 %s]] % {output, table.concat(sourcepath, ":"), classpath, table.concat(jars, ":"), table.concat(source_files, " ")} );
        end
    end
end

function JavaPrototype.clean( java )
    for dependency in java:get_dependencies() do
        if dependency:prototype() == nil then
            rm( dependency:path() );
        end
    end
end

function Java( definition )
    build.begin_target();
    return build.end_target( function()
        local java;
        local settings = build.push_settings( definition.settings );
        if build.built_for_platform_and_variant(settings) then
            java = target( "", JavaPrototype, definition );
            java.settings = settings;

            for _, value in ipairs(java) do
                if type(value) == "string" then
                    local source_file = file( value );
                    source_file:set_required_to_exist( true );
                    source_file.unit = java;
                    source_file.settings = settings;

                    local class_file = file( "%s/%s.class" % {settings.classes, build.strip(value)} );
                    class_file.source = value;
                    java.class_file = class_file;
                    class_file:add_dependency( source_file );
                    class_file:add_dependency( directory );
                    java:add_dependency( class_file );

                    local directory = Directory( branch(class_file:get_filename()) );
                    class_file:add_dependency( directory );
                elseif type(value) == "function" then
                    local dependency = build.expand_target( value );
                    java:add_dependency( dependency );
                end
            end
        end        
        build.pop_settings();
        return java;
    end);
end
