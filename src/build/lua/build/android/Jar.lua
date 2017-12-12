
local Jar = build.TargetPrototype( "Jar" );

function Jar.create( settings, id )
    local jar = build.Target( id, Jar );
    jar:set_filename( ("%s/%s"):format(settings.lib, id) );
    jar.settings = settings;
    build.add_jar_dependencies( jar, settings.jars );
    working_directory():add_dependency( jar );
    return jar;
end

function Jar.call( jar, definition )
    build.add_jar_dependencies( java, definition.jars );
    for _, dependency in ipairs(definition) do 
        jar:add_dependency( dependency );
        dependency.module = jar;
    end
end

local function included( jar, filename )
    if is_directory(filename) then 
        return false;
    end

    if jar.excludes then 
        for _, pattern in ipairs(jar.excludes) do 
            if string.match(filename, pattern) then 
                return false;
            end
        end
    end

    if jar.includes then 
        for _, pattern in ipairs(jar.includes) do 
            if string.match(filename, pattern) then 
                return true;
            end
        end
        return false;
    end

    return true;
end

function Jar.build( jar )
    if jar:outdated() then
        print( leaf(jar:filename()) );
        local jar_ = native( ("%s/bin/jar"):format(jar.settings.android.jdk_directory) );
        local directory = build.classes_directory( jar );
        pushd( directory );

        local classes = {};
        for filename in find(".") do 
            if included(jar, filename) then
                table.insert( classes, relative(filename) );
            end
        end

        build.system( jar_, ('jar cvf "%s" "%s"'):format(jar:filename(), table.concat(classes, [[" "]])) );
        popd();
    end    
end

function Jar.clean( jar )
    rm( jar:filename() );
end

android.Jar = Jar;
