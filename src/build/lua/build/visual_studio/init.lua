
module( "build.visual_studio", package.seeall );

require "build.fs";
require "build.visual_studio.vcxproj";
require "build.visual_studio.sln";

-- Generate a UUID by calling the uuidgen tool.
local function uuid()    
    local uuids = {};
    local uuidgen = ("%s/bin/x64/uuidgen.exe"):format( settings.msvc.windows_sdk_directory );
    local arguments = "uuidgen";
    build.system( uuidgen, arguments, function(line)
        local uuid = line:match( "[%w-_]+" );
        if uuid then 
            table.insert( uuids, uuid );
        end
    end );
    assert( uuids[1], "UUID generation failed!" );
    return upper( uuids[1] );
end

local function find_projects( target, projects )
	local prototype = target:prototype();
	if prototype == build.Executable or prototype == build.StaticLibrary or prototype == build.DynamicLibrary then 
		table.insert( projects, target );
	end
	for dependency in target:dependencies() do 
		find_projects( dependency, projects );
	end
end

function solution()
    platform = "";
    variant = "";
    build.load();    
    local all = find_target( root() );
    assertf( all, "No target found at '%s' to generate Visual Studio solution", root() );
    assertf( build.settings.sln, "The solution filename is not specified by settings.sln" );

    local projects = {};
    find_projects( all, projects );

    local fs = build.fs;
    for _, project in ipairs(projects) do 
    	project.uuid = uuid();
    	pushd( project:working_directory():path() );
		local DEFAULT_SOURCE = { "^.*%.cp?p?$", "^.*%.hp?p?$", "^.*%.mm?$", "^.*%.java$" };
		local files = fs.ls( pwd(), DEFAULT_SOURCE );
		build.visual_studio.vcxproj.generate( project, files );
		popd();
    end

    build.visual_studio.sln.generate( build.settings.sln, projects );
end

_G.sln = solution;
