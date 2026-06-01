
SUITE 'directory-function-tests';

-- pwd()

TEST_FIXTURE(_unit_test_setup, 'pwd_returns_absolute_path', function()
    local path = pwd();
    CHECK( is_absolute(path) );
end);

-- cd()

TEST_FIXTURE(_unit_test_setup, 'cd_changes_pwd_and_back_via_absolute', function()
    local path = pwd();
    cd( 'cd_test_subdir' );
    CHECK_EQUAL( 'cd_test_subdir', leaf(pwd()) );
    cd( path );
    CHECK_EQUAL( path, pwd() );
end);

TEST_FIXTURE(_unit_test_setup, 'cd_relative_resolves_against_current_directory', function()
    cd( 'one' );
    cd( 'two' );
    CHECK_EQUAL( 'two', leaf(pwd()) );
    CHECK_EQUAL( 'one', leaf(branch(pwd())) );
end);

TEST_FIXTURE(_unit_test_setup, 'cd_with_absolute_path_sets_pwd_directly', function()
    local target = root( 'cd_absolute_target' );
    cd( target );
    CHECK_EQUAL( target, pwd() );
end);

-- pushd(), popd()

TEST_FIXTURE(_unit_test_setup, 'pushd_popd_round_trip', function()
    local before = pwd();
    pushd( 'a' );
    local after_first = pwd();
    CHECK_EQUAL( 'a', leaf(after_first) );
    pushd( 'b' );
    CHECK_EQUAL( 'b', leaf(pwd()) );
    CHECK_EQUAL( 'a', leaf(branch(pwd())) );
    popd();
    CHECK_EQUAL( after_first, pwd() );
    popd();
    CHECK_EQUAL( before, pwd() );
end);

TEST_FIXTURE(_unit_test_setup, 'pushd_relative_resolves_against_current_directory', function()
    pushd( 'outer' );
    pushd( 'inner' );
    CHECK_EQUAL( 'inner', leaf(pwd()) );
    CHECK_EQUAL( 'outer', leaf(branch(pwd())) );
    popd();
    popd();
end);

TEST_FIXTURE(_unit_test_setup, 'popd_on_single_entry_stack_is_silent_noop', function()
    local before = pwd();
    popd();
    CHECK_EQUAL( before, pwd() );
end);
