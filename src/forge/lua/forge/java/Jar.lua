
local Jar = build:TargetPrototype( "Jar" );

local function included( jar, filename )
    if build:is_directory(filename) then 
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

function Jar.build( build, target )
    local settings = target.settings;
    local jar = build:native( ("%s/bin/jar"):format(settings.java.jdk_directory) );
    local directory = settings.classes_directory( target );
    build:pushd( directory );

    local classes = {};
    for filename in build:find(".") do 
        if included(target, filename) then
            table.insert( classes, build:relative(filename) );
        end
    end

    build:system( jar, ('jar cvf "%s" "%s"'):format(target:filename(), table.concat(classes, [[" "]])) );
    build:popd();
end

java.Jar = Jar;
