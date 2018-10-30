
local Jar = forge:FilePrototype( 'Jar' );

local function included( jar, filename )
    if forge:is_directory(filename) then 
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

function Jar.build( forge, target )
    local settings = target.settings;
    local jar = forge:native( ("%s/bin/jar"):format(settings.java.jdk_directory) );
    local directory = settings.classes_directory( target );
    forge:pushd( directory );

    local classes = {};
    for filename in forge:find(".") do 
        if included(target, filename) then
            table.insert( classes, forge:relative(filename) );
        end
    end

    forge:system( jar, ('jar cvf "%s" "%s"'):format(target:filename(), table.concat(classes, [[" "]])) );
    forge:popd();
end
