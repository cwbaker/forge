
local forge = require( 'forge' ):load();
local cat = '/usr/bin/cat';

local function ignore_filter()
end

local function check_accesses( expected_accesses )
    local index = 1;
    local function dependencies_filter( line )        
        local access, path = decode_access( line );
        local expected_access = expected_accesses[index];
        CHECK_EQUAL( expected_access[1], access );
        CHECK_EQUAL( expected_access[2], path );
    end
    return dependencies_filter, ignore_filter, ignore_filter;
end

local function check_no_accesses()
    local function dependencies_filter()
        CHECK( false );
    end
    return dependencies_filter, ignore_filter, ignore_filter;
end

-- Relative path within source tree.
run( cat, {'cat', relative('hooks.lua')}, nil, check_accesses {
    {'read', absolute('hooks.lua')};
} );

-- Absolute path within source tree.
run( cat, {'cat', absolute('hooks.lua')}, nil, check_accesses {
    {'read', absolute('hooks.lua')};
} );

-- relative_path_outside_source_tree = function()
-- end;

-- absolute_path_outside_source_tree = function()
-- end;

-- Ignore_non_existing_file.
local exit_code = execute( cat, 'cat this-file-does-not-exist', nil, check_no_accesses() );
CHECK( exit_code ~= 0 );

-- ignore_temporary_file = function()
-- end
