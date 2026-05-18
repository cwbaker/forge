
require 'forge';

local function open_files_for_hooks_executable()
    if operating_system() == 'windows' then
        return executable( 'forge_test_open_files_for_hooks.exe' );
    end
    return executable( 'forge_test_open_files_for_hooks' );
end

local function ignore_filter()
end

local function hooks_reports_reads_of_opened_files()
    local accesses = 0;
    local expected_path = absolute( 'hooks_tests.lua' );

    local function dependencies_filter( line )
        accesses = accesses + 1;
        local access, path = decode_access( line );
        assert( access == 'read', ('Expected access "read", got "%s"'):format(access) );
        assert( absolute(path) == expected_path, ('Expected path "%s", got "%s"'):format(expected_path, absolute(path)) );
        assert( leaf(path) ~= 'this-file-does-not-exist', 'Missing file reported by hooks' );
    end

    local open_files_for_hooks = open_files_for_hooks_executable();
    local arguments = table.concat({
        open_files_for_hooks;
        'hooks_tests.lua';
        absolute('hooks_tests.lua');
        'this-file-does-not-exist';
        absolute('this-file-does-not-exist');
    }, ' ');

    run( open_files_for_hooks, arguments, nil, dependencies_filter, ignore_filter, ignore_filter );
    assert( accesses > 0, 'No hook accesses reported' );
end

local function hooks_reports_reads_of_non_ascii_paths()
    local NON_ASCII_FILE = 'h\xc3\xa9llo.txt';
    local expected_path = absolute( NON_ASCII_FILE );
    local saw_read = false;

    create( NON_ASCII_FILE );

    local function dependencies_filter( line )
        local access, path = decode_access( line );
        if access == 'read' and absolute(path) == expected_path then
            saw_read = true;
        end
    end

    local open_files_for_hooks = open_files_for_hooks_executable();
    local arguments = table.concat({
        open_files_for_hooks;
        'hooks_tests.lua';
        absolute('hooks_tests.lua');
        NON_ASCII_FILE;
        expected_path;
    }, ' ');

    run( open_files_for_hooks, arguments, nil, dependencies_filter, ignore_filter, ignore_filter );
    assert( saw_read, ('Hooks did not report read of "%s"'):format(expected_path) );
end

hooks_reports_reads_of_opened_files();
hooks_reports_reads_of_non_ascii_paths();
