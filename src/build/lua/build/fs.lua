
local fs = {};

local function filter( filename, includes, excludes )
    if is_directory(filename) then 
        return false;
    end
    if excludes then 
        for _, pattern in ipairs(excludes) do 
            if string.match(filename, pattern) then 
                return false;
            end
        end
    end
    if includes then 
        for _, pattern in ipairs(includes) do 
            if string.match(filename, pattern) then 
                return true;
            end
        end
        return false;
    end
    return true;
end

function fs.ls( path, includes, excludes )
	local files = {};
	for filename in _G.ls(path or pwd()) do 
		if filter(filename, includes, excludes) then
			table.insert( files, filename );
		end
	end
	return files;
end

function fs.find( path, includes, excludes )
    local files = {};
    for filename in _G.find(path or pwd()) do 
        if filter(filename, includes, excludes) then
            table.insert( files, filename );
        end
    end
    return files;
end

return fs;
