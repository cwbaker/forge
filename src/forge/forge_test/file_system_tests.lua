
TestSuite {
    -- pwd(), cd(), pushd(), popd()
    pwd_returns_absolute_path = function()
        local path = pwd();
        CHECK( is_absolute(path) );
    end;

    cd_changes_pwd_and_back_via_absolute = function()
        local path = pwd();
        cd( 'cd_test_subdir' );
        CHECK_EQUAL( 'cd_test_subdir', leaf(pwd()) );
        cd( path );
        CHECK_EQUAL( path, pwd() );
    end;

    pushd_popd_round_trip = function()
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
    end;

    -- exists()
    exists_true_for_existing_file = function()
        create( 'exists_true_for_existing_file.tmp' );
        CHECK( exists(root('exists_true_for_existing_file.tmp')) );
    end;

    exists_false_for_missing_file = function()
        CHECK( not exists(root('definitely_does_not_exist_aaaaaa.tmp')) );
    end;

    -- is_file(), is_directory()
    is_file_and_is_directory_discriminate = function()
        create( 'is_file_and_is_directory_discriminate.tmp' );
        local file_path = root( 'is_file_and_is_directory_discriminate.tmp' );
        CHECK( is_file(file_path) );
        CHECK( not is_directory(file_path) );
        CHECK( is_directory(root()) );
        CHECK( not is_file(root()) );
    end;

    -- rm()
    rm_removes_file = function()
        create( 'rm_removes_file.tmp' );
        local path = root( 'rm_removes_file.tmp' );
        CHECK( exists(path) );
        rm( path );
        CHECK( not exists(path) );
    end;

    -- mkdir(), rmdir()
    mkdir_creates_directory_rmdir_removes_it = function()
        local directory = root( 'mkdir_rmdir_round_trip_dir' );
        mkdir( directory );
        CHECK( is_directory(directory) );
        rmdir( directory );
        CHECK( not exists(directory) );
    end;

    mkdir_creates_parent_directories = function()
        local directory = root( 'mkdir_parents/level1/level2' );
        mkdir( directory );
        CHECK( is_directory(directory) );
        CHECK( is_directory(root('mkdir_parents/level1')) );
        rmdir( root('mkdir_parents') );
        CHECK( not exists(root('mkdir_parents')) );
    end;

    -- cp()
    cp_copies_file_contents = function()
        create( 'cp_src.tmp', nil, 'hello' );
        local source = root( 'cp_src.tmp' );
        local destination = root( 'cp_dst.tmp' );
        cp( destination, source );
        CHECK( exists(destination) );
        CHECK( is_file(destination) );
        rm( destination );
    end;

    -- ls()
    ls_enumerates_directory_entries = function()
        local directory = root( 'ls_dir' );
        mkdir( directory );
        create( 'ls_dir/a.txt' );
        create( 'ls_dir/b.txt' );
        local found = {};
        for entry in ls( directory ) do
            found[leaf(entry)] = true;
            CHECK( is_absolute(entry) );
        end
        CHECK( found['a.txt'] );
        CHECK( found['b.txt'] );
        rm( root('ls_dir/a.txt') );
        rm( root('ls_dir/b.txt') );
        rmdir( directory );
    end;

    -- find()
    find_recursively_enumerates_nested_entries = function()
        local directory = root( 'find_dir' );
        mkdir( directory );
        mkdir( root('find_dir/nested') );
        create( 'find_dir/a.txt' );
        create( 'find_dir/nested/b.txt' );
        local found = {};
        for entry in find( directory ) do
            found[leaf(entry)] = true;
        end
        CHECK( found['a.txt'] );
        CHECK( found['b.txt'] );
        rm( root('find_dir/a.txt') );
        rm( root('find_dir/nested/b.txt') );
        rmdir( root('find_dir/nested') );
        rmdir( directory );
    end;

    -- touch()
    touch_preserves_file = function()
        create( 'touch_preserves_file.tmp', 1000 );
        touch( 'touch_preserves_file.tmp', 5000 );
        CHECK( exists(root('touch_preserves_file.tmp')) );
    end;
};
