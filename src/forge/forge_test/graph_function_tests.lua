
-- Tests for the graph functions documented in
-- docs/reference/graph-functions.md.  The traversal functions postorder()
-- and preorder() have their own suites in postorder_tests.lua and
-- preorder.lua; this suite covers the remaining graph functions.
--
-- Note: graph-functions.md documents add_target() and add_rule() but neither
-- is exposed to Lua.  Targets and rules are created through Target() and
-- Rule() (and looked up with find_target()), which is what these tests use.

SUITE 'graph-function-tests';

-- find_target()

TEST_FIXTURE(_unit_test_setup, 'find_target_returns_nil_for_missing_target', function()
    CHECK( find_target('find_target_missing') == nil );
end);

TEST_FIXTURE(_unit_test_setup, 'find_target_returns_existing_target', function()
    local created = Target( forge, 'find_target_existing' );
    CHECK( find_target('find_target_existing') == created );
end);

-- anonymous()

TEST_FIXTURE(_unit_test_setup, 'anonymous_generates_unique_identifiers', function()
    local first = anonymous();
    local second = anonymous();
    CHECK( first ~= '' );
    CHECK( first ~= second );
end);

-- working_directory()

TEST_FIXTURE(_unit_test_setup, 'working_directory_is_parent_of_created_targets', function()
    local directory = working_directory();
    CHECK( directory ~= nil );
    local target = Target( forge, 'working_directory_child' );
    CHECK( target:parent() == directory );
end);

-- current_buildfile()

TEST_FIXTURE(_unit_test_setup, 'current_buildfile_is_available_within_a_buildfile', function()
    current_buildfile_probe = nil;
    create( 'current_buildfile_probe.forge', nil, 'current_buildfile_probe = current_buildfile();' );
    buildfile( 'current_buildfile_probe.forge' );
    CHECK( current_buildfile_probe ~= nil );
end);

-- buildfile()

TEST_FIXTURE(_unit_test_setup, 'buildfile_returns_zero_on_success', function()
    create( 'graph_function_buildfile.forge', nil, '' );
    local errors = buildfile( 'graph_function_buildfile.forge' );
    CHECK_EQUAL( 0, errors );
end);

TEST_FIXTURE(_unit_test_setup, 'buildfile_reports_errors_for_missing_buildfile', function()
    quiet();
    local errors = buildfile( 'graph_function_missing.forge' );
    CHECK( errors > 0 );
end);

-- clear()

TEST_FIXTURE(_unit_test_setup, 'clear_clears_dependencies', function()
    local a = Target( forge, 'clear_a' );
    local b = Target( forge, 'clear_b' );
    a:add_dependency( b );
    CHECK( a:dependency(1) == b );
    clear();
    local a_after = find_target( 'clear_a' );
    CHECK( a_after == nil or a_after:dependency(1) == nil );
end);

-- save_binary(), load_binary()

TEST_FIXTURE(_unit_test_setup, 'save_binary_writes_to_filename_set_by_load_binary', function()
    local cache = root( '.forge_graph_function_cache' );
    if exists(cache) then rm( cache ); end
    -- load_binary() is the only way to set the filename that save_binary()
    -- writes to; the file does not need to exist yet.
    load_binary( cache );
    Target( forge, 'save_binary_target.txt', Rule('File') );
    save_binary();
    CHECK( exists(cache) );
    rm( cache );
end);

-- prune()

TEST_FIXTURE(_unit_test_setup, 'prune_stops_preorder_descent_into_dependencies', function()
    local parent = Target( forge, 'prune_parent' );
    local child = Target( forge, 'prune_child' );
    parent:add_dependency( child );
    local visited = {};
    preorder( parent, function(target)
        visited[target:id()] = true;
        if target:id() == 'prune_parent' then
            prune();
        end
    end );
    CHECK( visited['prune_parent'] );
    CHECK( not visited['prune_child'] );
end);
