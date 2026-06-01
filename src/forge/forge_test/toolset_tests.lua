
-- Tests for the toolset construction and methods documented in
-- docs/reference/toolset.md.
--
-- Note: toolset.md documents create(), append(), and merge() as toolset
-- methods.  create() and append() are not implemented, and merge() is a
-- global function rather than a toolset method; the global merge() is tested
-- here as the documented behaviour.

-- A fresh toolset per test - the unit test fixture clears the graph between
-- tests so the toolset is rebuilt each time rather than shared.
local function make_toolset()
    local forge = require( 'forge' ):load();
    return forge.Toolset() {};
end

SUITE 'toolset-tests';

-- Toolset() construction

TEST_FIXTURE(_unit_test_setup, 'toolset_construction_returns_a_toolset', function()
    local toolset = make_toolset();
    CHECK( toolset ~= nil );
end);

-- clone()

TEST_FIXTURE(_unit_test_setup, 'clone_copies_settings_with_overrides', function()
    local toolset = make_toolset();
    toolset.value = 'original';
    toolset.kept = 'kept';
    local cloned = toolset:clone{ value = 'override' };
    CHECK_EQUAL( 'override', cloned.value );
    CHECK_EQUAL( 'kept', cloned.kept );
    CHECK_EQUAL( 'original', toolset.value );
end);

-- inherit()

TEST_FIXTURE(_unit_test_setup, 'inherit_inherits_via_metatable_with_overrides', function()
    local toolset = make_toolset();
    toolset.base_value = 'base';
    local child = toolset:inherit{ own_value = 'own' };
    CHECK_EQUAL( 'own', child.own_value );
    CHECK_EQUAL( 'base', child.base_value );
end);

-- defaults()

TEST_FIXTURE(_unit_test_setup, 'defaults_only_sets_missing_keys', function()
    local toolset = make_toolset();
    toolset.existing = 'keep';
    toolset:defaults{ existing = 'ignored'; added = 'new' };
    CHECK_EQUAL( 'keep', toolset.existing );
    CHECK_EQUAL( 'new', toolset.added );
end);

-- merge() (documented as a toolset method but implemented as a global)

TEST_FIXTURE(_unit_test_setup, 'merge_copies_string_keyed_fields', function()
    local destination = { existing = 'keep' };
    local source = { added = 'value'; existing = 'overwritten' };
    merge( destination, source );
    CHECK_EQUAL( 'value', destination.added );
    CHECK_EQUAL( 'overwritten', destination.existing );
end);

-- interpolate()

TEST_FIXTURE(_unit_test_setup, 'interpolate_substitutes_from_variables', function()
    local toolset = make_toolset();
    CHECK_EQUAL( 'hello world', toolset:interpolate('${greeting} world', {greeting = 'hello'}) );
end);

TEST_FIXTURE(_unit_test_setup, 'interpolate_substitutes_from_toolset_settings', function()
    local toolset = make_toolset();
    toolset.greeting = 'hi';
    CHECK_EQUAL( 'hi there', toolset:interpolate('${greeting} there') );
end);

-- platform_matches()

TEST_FIXTURE(_unit_test_setup, 'platform_matches_empty_platform_matches_everything', function()
    local toolset = make_toolset();
    toolset.platform = '';
    CHECK( toolset:platform_matches('anything') );
end);

TEST_FIXTURE(_unit_test_setup, 'platform_matches_compares_platform_setting', function()
    local toolset = make_toolset();
    toolset.platform = 'linux';
    CHECK( toolset:platform_matches('linux') );
    CHECK( toolset:platform_matches('lin') );
    CHECK( toolset:platform_matches('win32', 'linux') );
    CHECK( not toolset:platform_matches('macos') );
    CHECK( not toolset:platform_matches('win32', 'macos') );
end);

-- which()

TEST_FIXTURE(_unit_test_setup, 'which_finds_existing_file_and_misses_absent_one', function()
    local toolset = make_toolset();
    if operating_system() ~= 'windows' then
        CHECK_EQUAL( '/bin/sh', toolset:which('/bin/sh') );
    end
    CHECK( toolset:which('/this/executable/does/not/exist/xyzzy') == nil );
end);

-- all()

TEST_FIXTURE(_unit_test_setup, 'all_returns_all_target_with_dependencies', function()
    local toolset = make_toolset();
    local a = Target( toolset, 'all_dep_a' );
    local all = toolset:all{ a };
    CHECK_EQUAL( 'all', all:id() );
    CHECK( all:dependency(1) == a );
end);

-- Target()

TEST_FIXTURE(_unit_test_setup, 'toolset_target_creates_target', function()
    local toolset = make_toolset();
    local target = toolset:Target( 'toolset_target.txt' );
    CHECK( target ~= nil );
    CHECK_EQUAL( 'toolset_target.txt', target:id() );
end);

-- File()

TEST_FIXTURE(_unit_test_setup, 'toolset_file_creates_cleanable_file_target', function()
    local toolset = make_toolset();
    local file = toolset:File( 'toolset_file_dir/out.txt' );
    CHECK( file:cleanable() );
    CHECK_EQUAL( file:path(), file:filename() );
end);

-- SourceFile()

TEST_FIXTURE(_unit_test_setup, 'toolset_source_file_creates_non_cleanable_target', function()
    local toolset = make_toolset();
    local source = toolset:SourceFile( 'toolset_source.txt' );
    CHECK( not source:cleanable() );
    CHECK_EQUAL( source:path(), source:filename() );
end);

-- Directory()

TEST_FIXTURE(_unit_test_setup, 'toolset_directory_creates_directory_target', function()
    local toolset = make_toolset();
    local directory = toolset:Directory( root('toolset_dir') );
    CHECK( directory ~= nil );
    CHECK( directory:cleanable() );
end);

-- Copy()

TEST_FIXTURE(_unit_test_setup, 'toolset_copy_creates_copy_target_per_source', function()
    local toolset = make_toolset();
    local outputs = toolset:Copy 'toolset_copied/%1' { 'copy_src.txt' };
    CHECK_EQUAL( 1, #outputs );
end);

-- cpdir()

TEST_FIXTURE(_unit_test_setup, 'cpdir_recursively_copies_files', function()
    local toolset = make_toolset();
    mkdir( root('cpdir_src/nested') );
    create( 'cpdir_src/a.txt' );
    create( 'cpdir_src/nested/b.txt' );
    toolset:cpdir( root('cpdir_dst'), root('cpdir_src') );
    CHECK( exists(root('cpdir_dst/a.txt')) );
    CHECK( exists(root('cpdir_dst/nested/b.txt')) );
    rmdir( root('cpdir_src') );
    rmdir( root('cpdir_dst') );
end);

-- dependencies_filter(), filenames_filter()
--
-- These return filter functions for execute() that detect dependencies and
-- output filenames from build-hook output lines.  Their end-to-end behaviour
-- through a real process is exercised in hooks_tests.lua; here we feed hook
-- lines to the returned filter directly.

TEST_FIXTURE(_unit_test_setup, 'dependencies_filter_adds_read_files_as_implicit_dependencies', function()
    local toolset = make_toolset();
    local target = Target( toolset, 'dependencies_filter_target' );
    create( 'dependencies_filter_input.txt' );
    local filter = toolset:dependencies_filter( target );
    filter( ("== read '%s'"):format(root('dependencies_filter_input.txt')) );
    local found = false;
    for _, dependency in target:all_dependencies() do
        if leaf(dependency:path()) == 'dependencies_filter_input.txt' then
            found = true;
        end
    end
    CHECK( found );
end);

TEST_FIXTURE(_unit_test_setup, 'filenames_filter_adds_written_files_as_filenames', function()
    local toolset = make_toolset();
    local target = toolset:File( 'filenames_filter_dir/out.bin' );
    local filter = toolset:filenames_filter( target );
    filter( ("== write '%s'"):format(root('filenames_filter_dir/generated.o')) );
    local found = false;
    for _, filename in target:filenames() do
        if leaf(filename) == 'generated.o' then
            found = true;
        end
    end
    CHECK( found );
end);
