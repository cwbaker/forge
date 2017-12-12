
JavaPrototype = TargetPrototype { "Java" };

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
            local output = "%s/classes" % obj_directory( java );
            local sourcepath = table.concat( java.sourcepath or {}, settings.path_separator );
            local classpath = output;
            local libraries = {
                "%s/platforms/%s/android.jar" % { settings.android.sdk_directory, settings.android.sdk_platform };
            };
            if settings.libraries then 
                for _, library in ipairs(settings.libraries) do 
                    table.insert( libraries, library );
                end
            end
            pushd( java.directory );
            build.system( javac, [[javac -Xlint:unchecked -d "%s" -sourcepath "%s" -classpath "%s" -target 1.5 -bootclasspath "%s" -encoding UTF-8 -g -source 1.5 %s]] % {output, sourcepath, classpath, table.concat(libraries, ":"), table.concat(source_files, " ")} );
            popd();
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

            local directory = Directory( "%s/classes/%s" % {obj_directory(java), java.package} );

            for _, value in ipairs(java) do
                if type(value) == "string" then
                    local source_file = file( "%s/%s" % {java.directory, value} );
                    source_file:set_required_to_exist( true );
                    source_file.unit = java;
                    source_file.settings = settings;

                    local class_file = file( "%s/classes/%s/%s.class" % {obj_directory(java), java.package, build.strip(value)} );
                    class_file.source = value;
                    java.class_file = class_file;
                    class_file:add_dependency( source_file );
                    class_file:add_dependency( directory );
                    java:add_dependency( class_file );
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
