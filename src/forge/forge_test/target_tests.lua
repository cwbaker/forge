
-- Tests for the Target methods documented in docs/reference/target.md.  The
-- outdated calculation is covered in depth by graph_tests.lua; this suite
-- focuses on identity, filenames, state flags, and the dependency accessors.

SUITE 'target-tests';

-- id(), path(), branch()

TEST_FIXTURE(_unit_test_setup, 'id_path_and_branch', function()
    local target = Target( forge, 'identity_dir/identity.txt' );
    CHECK_EQUAL( 'identity.txt', target:id() );
    CHECK_EQUAL( 'identity.txt', leaf(target:path()) );
    CHECK_EQUAL( 'identity_dir', leaf(branch(target:path())) );
    CHECK( target:branch():find('identity_dir', 1, true) ~= nil );
end);

-- rule()

TEST_FIXTURE(_unit_test_setup, 'rule_returns_creation_rule_or_nil', function()
    local File = Rule( 'File' );
    local with_rule = Target( forge, 'with_rule.txt', File );
    CHECK( with_rule:rule() == File );
    local without_rule = Target( forge, 'without_rule.txt' );
    CHECK( without_rule:rule() == nil );
end);

-- parent()

TEST_FIXTURE(_unit_test_setup, 'parent_is_namespace_parent', function()
    local child = Target( forge, 'parent_dir/child.txt' );
    local parent = child:parent();
    CHECK( parent ~= nil );
    CHECK_EQUAL( 'parent_dir', parent:id() );
end);

-- working_directory(), set_working_directory()

TEST_FIXTURE(_unit_test_setup, 'set_working_directory_changes_working_directory', function()
    local target = Target( forge, 'wd_target.txt' );
    local directory = Target( forge, 'wd_dir' );
    target:set_working_directory( directory );
    CHECK( target:working_directory() == directory );
end);

-- set_cleanable(), cleanable()

TEST_FIXTURE(_unit_test_setup, 'set_cleanable_toggles_cleanable', function()
    local target = Target( forge, 'cleanable.txt' );
    target:set_cleanable( true );
    CHECK( target:cleanable() );
    target:set_cleanable( false );
    CHECK( not target:cleanable() );
end);

-- set_built(), built()

TEST_FIXTURE(_unit_test_setup, 'set_built_toggles_built', function()
    local target = Target( forge, 'built.txt' );
    target:set_built( true );
    CHECK( target:built() );
    target:set_built( false );
    CHECK( not target:built() );
end);

-- timestamp(), last_write_time()

TEST_FIXTURE(_unit_test_setup, 'timestamp_and_last_write_time_are_numbers', function()
    local target = Target( forge, 'timestamp_target.txt' );
    target:set_filename( target:path() );
    create( 'timestamp_target.txt', 1234 );
    postorder( target, function() end );
    CHECK( type(target:timestamp()) == 'number' );
    CHECK( type(target:last_write_time()) == 'number' );
end);

-- set_filename(), filename(), add_filename(), clear_filenames()

TEST_FIXTURE(_unit_test_setup, 'set_add_get_and_clear_filenames', function()
    local target = Target( forge, 'filenames_target' );
    target:set_filename( '/path/one.txt' );
    CHECK_EQUAL( '/path/one.txt', target:filename() );
    CHECK_EQUAL( '/path/one.txt', target:filename(1) );
    target:add_filename( '/path/two.txt' );
    CHECK_EQUAL( '/path/two.txt', target:filename(2) );
    target:clear_filenames();
    CHECK_EQUAL( '', target:filename() );
end);

-- filenames()

TEST_FIXTURE(_unit_test_setup, 'filenames_iterates_all_filenames', function()
    local target = Target( forge, 'filenames_iter' );
    target:add_filename( '/a.txt' );
    target:add_filename( '/b.txt' );
    local collected = {};
    for _, filename in target:filenames() do
        collected[#collected + 1] = filename;
    end
    CHECK_EQUAL( 2, #collected );
    CHECK_EQUAL( '/a.txt', collected[1] );
    CHECK_EQUAL( '/b.txt', collected[2] );
end);

-- directory()

TEST_FIXTURE(_unit_test_setup, 'directory_returns_branch_of_filename', function()
    local target = Target( forge, 'directory_target' );
    target:set_filename( '/some/dir/file.txt' );
    CHECK_EQUAL( '/some/dir', target:directory(1) );
end);

-- add_dependency(), dependency()

TEST_FIXTURE(_unit_test_setup, 'add_and_get_explicit_dependencies', function()
    local target = Target( forge, 'dep_target' );
    local a = Target( forge, 'dep_a' );
    local b = Target( forge, 'dep_b' );
    target:add_dependency( a );
    target:add_dependency( b );
    CHECK( target:dependency(1) == a );
    CHECK( target:dependency(2) == b );
    CHECK( target:dependency(3) == nil );
end);

TEST_FIXTURE(_unit_test_setup, 'add_dependency_ignores_self_and_nil', function()
    local target = Target( forge, 'self_dep' );
    target:add_dependency( target );
    target:add_dependency( nil );
    CHECK( target:dependency(1) == nil );
end);

-- dependencies()

TEST_FIXTURE(_unit_test_setup, 'dependencies_iterates_explicit_dependencies', function()
    local target = Target( forge, 'dep_iter' );
    local a = Target( forge, 'dep_iter_a' );
    local b = Target( forge, 'dep_iter_b' );
    target:add_dependency( a );
    target:add_dependency( b );
    local collected = {};
    for _, dependency in target:dependencies() do
        collected[#collected + 1] = dependency;
    end
    CHECK_EQUAL( 2, #collected );
    CHECK( collected[1] == a );
    CHECK( collected[2] == b );
end);

-- remove_dependency()

TEST_FIXTURE(_unit_test_setup, 'remove_dependency_removes_dependency', function()
    local target = Target( forge, 'remove_target' );
    local a = Target( forge, 'remove_a' );
    target:add_dependency( a );
    CHECK( target:dependency(1) == a );
    target:remove_dependency( a );
    CHECK( target:dependency(1) == nil );
end);

-- add_ordering_dependency(), ordering_dependency()

TEST_FIXTURE(_unit_test_setup, 'add_ordering_dependency_and_index_access', function()
    local target = Target( forge, 'ord_target' );
    local a = Target( forge, 'ord_a' );
    target:add_ordering_dependency( a );
    CHECK( target:ordering_dependency(1) == a );
    CHECK( target:ordering_dependency(2) == nil );
end);

-- add_implicit_dependency(), clear_implicit_dependencies()

TEST_FIXTURE(_unit_test_setup, 'add_and_clear_implicit_dependencies', function()
    local target = Target( forge, 'imp_target' );
    local a = Target( forge, 'imp_a' );
    target:add_implicit_dependency( a );
    local function depends_on_a()
        for _, dependency in target:all_dependencies() do
            if dependency == a then return true; end
        end
        return false;
    end
    CHECK( depends_on_a() );
    target:clear_implicit_dependencies();
    CHECK( not depends_on_a() );
end);

-- add_passive_dependency()

TEST_FIXTURE(_unit_test_setup, 'add_passive_dependency_runs', function()
    local target = Target( forge, 'pas_target' );
    local a = Target( forge, 'pas_a' );
    -- Passive dependencies are not exposed through an accessor; check that the
    -- call succeeds and does not register an explicit dependency.
    target:add_passive_dependency( a );
    CHECK( target:dependency(1) == nil );
end);

-- all_dependencies()

TEST_FIXTURE(_unit_test_setup, 'all_dependencies_includes_explicit_and_ordering', function()
    local target = Target( forge, 'all_target' );
    local explicit = Target( forge, 'all_explicit' );
    local ordering = Target( forge, 'all_ordering' );
    target:add_dependency( explicit );
    target:add_ordering_dependency( ordering );
    local found_explicit, found_ordering = false, false;
    for _, dependency in target:all_dependencies() do
        if dependency == explicit then found_explicit = true; end
        if dependency == ordering then found_ordering = true; end
    end
    CHECK( found_explicit );
    CHECK( found_ordering );
end);
