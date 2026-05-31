
TestSuite {
    files_are_outdated_if_they_do_not_exist = function()
        local foo_cpp = Target( forge, 'outdated_missing_foo.cpp' );
        foo_cpp:set_filename( foo_cpp:path() );
        local foo_obj = Target( forge, 'outdated_missing_foo.obj' );
        foo_obj:set_filename( foo_obj:path() );
        foo_obj:add_dependency( foo_cpp );
        create( 'outdated_missing_foo.cpp' );
        postorder( foo_obj, function() end );
        CHECK( foo_obj:outdated() );
    end;

    files_are_not_outdated_if_they_exist = function()
        local SourceFile = Rule( 'SourceFile' );
        local File = Rule( 'File' );
        local foo_cpp = Target( nil, 'not_outdated_foo.cpp', SourceFile );
        foo_cpp:set_filename( foo_cpp:path() );
        local foo_obj = Target( nil, 'not_outdated_foo.obj', File );
        foo_obj:set_filename( foo_obj:path() );
        foo_obj:add_dependency( foo_cpp );
        create( 'not_outdated_foo.cpp' );
        create( 'not_outdated_foo.obj' );
        postorder( foo_obj, function() end );
        CHECK( foo_obj:outdated() == false );
    end;

    targets_are_outdated_if_their_dependencies_are_outdated = function()
        local foo_cpp = Target( forge, 'outdated_deps_foo.cpp' );
        foo_cpp:set_filename( foo_cpp:path() );
        local foo_hpp = Target( forge, 'outdated_deps_foo.hpp' );
        foo_hpp:set_filename( foo_hpp:path() );
        local foo_obj = Target( forge, 'outdated_deps_foo.obj' );
        foo_obj:set_filename( foo_obj:path() );
        foo_cpp:add_dependency( foo_hpp );
        foo_obj:add_dependency( foo_cpp );
        create( 'outdated_deps_foo.cpp', 1 );
        create( 'outdated_deps_foo.hpp', 2 );
        create( 'outdated_deps_foo.obj', 1 );
        postorder( foo_obj, function() end );
        CHECK( foo_obj:outdated() );
    end;

    creating_the_same_target_with_different_prototypes_fails = function()
        local SourceFile = Rule( 'SourceFile' );
        local File = Rule( 'File' );
        Target( forge, 'foo.cpp', SourceFile );
        Target( forge, 'foo.cpp', File );
        CHECK_EQUAL( "The target 'foo.cpp' has been created with rules 'SourceFile' and 'File'", error_message(0) );
    end;

    targets_are_children_of_the_working_directory_when_they_are_created = function()
        local File = Rule( 'File' );
        local foo_cpp = Target( forge, 'children_foo.cpp', File );
        CHECK( foo_cpp:parent() == foo_cpp:working_directory() );
    end;
};
