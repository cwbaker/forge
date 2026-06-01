
SUITE 'path-tests';
-- branch(), leaf(), basename(), extension()

TEST_FIXTURE(_unit_test_setup, 'branch_returns_parent', function()
    CHECK_EQUAL( 'foo/bar', branch('foo/bar/baz.txt') );
    CHECK_EQUAL( 'foo', branch('foo/bar') );
    CHECK_EQUAL( '', branch('bar') );
end);

TEST_FIXTURE(_unit_test_setup, 'branch_returns_parent_of_hidden_filename', function()
    CHECK_EQUAL( 'foo/bar', branch('foo/bar/.gitignore') );
    CHECK_EQUAL( 'bar', branch('bar/.gitignore') );
    CHECK_EQUAL( '', branch('.gitignore') );
end);

TEST_FIXTURE(_unit_test_setup, 'leaf_returns_filename', function()
    CHECK_EQUAL( 'baz.txt', leaf('foo/bar/baz.txt') );
    CHECK_EQUAL( 'bar', leaf('foo/bar') );
    CHECK_EQUAL( 'bar', leaf('bar') );
end);

TEST_FIXTURE(_unit_test_setup, 'leaf_returns_hidden_filename', function()
    CHECK_EQUAL( '.gitignore', leaf('foo/bar/.gitignore') );
    CHECK_EQUAL( '.gitignore', leaf('bar/.gitignore') );
    CHECK_EQUAL( '.gitignore', leaf('.gitignore') );
end);

TEST_FIXTURE(_unit_test_setup, 'basename_strips_extension', function()
    CHECK_EQUAL( 'baz', basename('foo/bar/baz.txt') );
    CHECK_EQUAL( 'baz', basename('baz.txt') );
    CHECK_EQUAL( 'baz', basename('baz') );
end);

TEST_FIXTURE(_unit_test_setup, 'basename_preserves_hidden_filename', function()
    CHECK_EQUAL( '.txt', basename('.txt') );
    CHECK_EQUAL( '.cache', basename('.cache') );
    CHECK_EQUAL( '.hidden', basename('.hidden') );
end);

TEST_FIXTURE(_unit_test_setup, 'extension_returns_dot_extension', function()
    CHECK_EQUAL( '.txt', extension('foo/bar/baz.txt') );
    CHECK_EQUAL( '.txt', extension('baz.txt') );
    CHECK_EQUAL( '', extension('baz') );
end);

TEST_FIXTURE(_unit_test_setup, 'extension_returns_right_most_dot_extension_with_multiple_dots', function()
    CHECK_EQUAL( '.txt', extension('foo/bar.baz.txt') );
    CHECK_EQUAL( '.txt', extension('bar.baz.txt') );
end);

TEST_FIXTURE(_unit_test_setup, 'extension_empty_for_hidden_filenames', function()
    CHECK_EQUAL( '', extension('.txt') );
    CHECK_EQUAL( '', extension('.cache') );
    CHECK_EQUAL( '', extension('.hidden') );
end);

-- is_absolute(), is_relative()

TEST_FIXTURE(_unit_test_setup, 'is_absolute_detects_absolute_paths', function()
    CHECK( is_absolute(root()) );
    CHECK( not is_absolute('foo/bar') );
    CHECK( not is_absolute('') );
end);

TEST_FIXTURE(_unit_test_setup, 'is_relative_detects_relative_paths', function()
    CHECK( is_relative('foo/bar') );
    CHECK( not is_relative(root()) );
end);

TEST_FIXTURE(_unit_test_setup, 'is_absolute_and_is_relative_are_opposites_for_non_empty_paths', function()
    CHECK( is_absolute(root()) ~= is_relative(root()) );
    CHECK( is_absolute('a') ~= is_relative('a') );
end);

-- absolute()

TEST_FIXTURE(_unit_test_setup, 'absolute_with_base_prepends_base_to_relative_path', function()
    CHECK_EQUAL( '/base/foo/bar', absolute('foo/bar', '/base') );
end);

TEST_FIXTURE(_unit_test_setup, 'absolute_with_base_passes_through_absolute_input', function()
    CHECK_EQUAL( '/already/absolute', absolute('/already/absolute', '/base') );
end);

TEST_FIXTURE(_unit_test_setup, 'absolute_with_base_normalizes_dot_dot', function()
    CHECK_EQUAL( '/base/baz', absolute('foo/../baz', '/base') );
end);

TEST_FIXTURE(_unit_test_setup, 'absolute_without_base_uses_working_directory', function()
    local path = absolute( 'sub/file.txt' );
    CHECK( is_absolute(path) );
    CHECK_EQUAL( 'file.txt', leaf(path) );
    CHECK_EQUAL( 'sub', leaf(branch(path)) );
end);

TEST_FIXTURE(_unit_test_setup, 'absolute_without_base_uses_working_directory_after_pushd', function()
    pushd( pwd() );
    pushd( 'pushed-directory' );
    local path = absolute( 'sub/file.txt' );
    CHECK( is_absolute(path) );
    CHECK_EQUAL( 'file.txt', leaf(path) );
    CHECK_EQUAL( 'sub', leaf(branch(path)) );
    CHECK_EQUAL( 'pushed-directory', leaf(branch(branch(path))) );
    popd();
    popd();
end);

TEST_FIXTURE(_unit_test_setup, 'absolute_without_base_uses_working_directory_after_cd', function()
    pushd( pwd() );
    cd( 'changed-directory' );
    local path = absolute( 'sub/file.txt' );
    CHECK( is_absolute(path) );
    CHECK_EQUAL( 'file.txt', leaf(path) );
    CHECK_EQUAL( 'sub', leaf(branch(path)) );
    CHECK_EQUAL( 'changed-directory', leaf(branch(branch(path))) );
    popd();
end);

TEST_FIXTURE(_unit_test_setup, 'absolute_without_base_returns_working_directory_for_empty_path', function()
    local path = absolute();
    CHECK( is_absolute(path) );
end);

-- relative()

TEST_FIXTURE(_unit_test_setup, 'relative_returns_child_segment', function()
    CHECK_EQUAL( 'c', relative('/a/b/c', '/a/b') );
    CHECK_EQUAL( 'c/d', relative('/a/b/c/d', '/a/b') );
end);

TEST_FIXTURE(_unit_test_setup, 'relative_passes_through_when_base_is_empty', function()
    CHECK_EQUAL( '/foo/bar', relative('/foo/bar', '') );
end);

TEST_FIXTURE(_unit_test_setup, 'relative_without_base_uses_working_directory', function()
    local path = relative( pwd()..'/sibling' );
    CHECK_EQUAL( 'sibling', path );
end);

-- root()

TEST_FIXTURE(_unit_test_setup, 'root_without_argument_returns_root_directory', function()
    local path = root();
    CHECK( path ~= '' );
    CHECK( is_absolute(path) );
end);

TEST_FIXTURE(_unit_test_setup, 'root_with_relative_argument_resolves_against_root', function()
    local path = root( 'a/b/c.txt' );
    CHECK( is_absolute(path) );
    CHECK_EQUAL( 'c.txt', leaf(path) );
    CHECK_EQUAL( 'b', leaf(branch(path)) );
end);

TEST_FIXTURE(_unit_test_setup, 'root_passes_through_absolute_argument', function()
    CHECK_EQUAL( absolute('/already/absolute.txt'), root('/already/absolute.txt') );
end);

-- initial()

TEST_FIXTURE(_unit_test_setup, 'initial_returns_absolute_path', function()
    local path = initial();
    CHECK( path ~= '' );
    CHECK( is_absolute(path) );
end);

TEST_FIXTURE(_unit_test_setup, 'initial_with_relative_argument_resolves_against_initial', function()
    local path = initial( 'rel/path' );
    CHECK( is_absolute(path) );
    CHECK_EQUAL( 'path', leaf(path) );
end);

-- home()

TEST_FIXTURE(_unit_test_setup, 'home_returns_absolute_path', function()
    local path = home();
    CHECK( path ~= '' );
    CHECK( is_absolute(path) );
end);

TEST_FIXTURE(_unit_test_setup, 'home_with_relative_argument_resolves_against_home', function()
    local path = home( 'rel/path' );
    CHECK( is_absolute(path) );
    CHECK_EQUAL( 'path', leaf(path) );
end);

-- executable()

TEST_FIXTURE(_unit_test_setup, 'executable_returns_absolute_path', function()
    local path = executable();
    CHECK( path ~= '' );
    CHECK( is_absolute(path) );
end);

TEST_FIXTURE(_unit_test_setup, 'executable_with_relative_argument_resolves_against_executable', function()
    local path = executable( 'rel/path' );
    CHECK( is_absolute(path) );
    CHECK_EQUAL( 'path', leaf(path) );
end);
