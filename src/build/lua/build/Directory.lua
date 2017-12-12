
DirectoryPrototype = TargetPrototype { "Directory" };

function DirectoryPrototype.build( directory )
    if directory:is_outdated() then
        mkdir( directory:get_filename() );
    end
end

function Directory( id )
	local directory = target( id, DirectoryPrototype );
    directory:set_filename( directory:path() );
    return directory;
end
