
local Jar = build.TargetPrototype( "Jar" );

function Jar.create( settings, id )
    local jar = build.Target( id, Jar );
    jar:set_filename( ("%s/%s"):format(settings.lib, id) );
    jar.settings = settings;
    build.add_jar_dependencies( jar, settings.jars );
    return jar;
end

function Jar.call( jar, definition )
    build.add_jar_dependencies( java, definition.jars );
    for _, dependency in ipairs(definition) do 
        jar:add_dependency( dependency );
    end
end

local function included( jar, filename )
    if build.is_directory(filename) then 
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
        print( build.leaf(jar:filename()) );
        local jar_ = build.native( ("%s/bin/jar"):format(jar.settings.android.jdk_directory) );
        local directory = build.classes_directory( jar );
        build.pushd( directory );

        local classes = {};
        for filename in build.find(".") do 
            if included(jar, filename) then
                table.insert( classes, build.relative(filename) );
            end
        end

        build.system( jar_, ('jar cvf "%s" "%s"'):format(jar:filename(), table.concat(classes, [[" "]])) );
        build.popd();
    end    
end

function Jar.clean( jar )
    build.rm( jar:filename() );
end

android.Jar = Jar;