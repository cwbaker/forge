
local forge = require( 'forge' ):load();

local accesses = 0;

local function check_accesses( expected_access, expected_path )
    local function dependencies_filter( line )
        accesses = accesses + 1;
        local access, path = decode_access( line );
        CHECK_EQUAL( expected_access, access );
        CHECK_EQUAL( expected_path, path );
        CHECK( leaf(path) ~= 'this-file-does-not-exist', 'Missing file reported by hooks' );
    end

    local function ignore_filter()
    end

    return dependencies_filter, ignore_filter, ignore_filter;
end

local open_files_for_hooks = executable( 'forge_test_open_files_for_hooks' );
if operating_system() == 'windows' then
    open_files_for_hooks = executable( 'forge_test_open_files_for_hooks.exe' );
end

run(
    open_files_for_hooks,
    'forge_test_open_files_for_hooks',
    nil,
    check_accesses('read', absolute('hooks.lua'))
);
