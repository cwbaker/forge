
local function dependency_graph()
    local forge = require( 'forge' ):load();
    local toolset = forge.Toolset() {};
    local one = toolset:Target 'one' {'two'; 'three';};
    local two = toolset:Target 'two' {'four'; 'three';};
    local three = toolset:Target 'three' {};
    local four = toolset:Target 'four' {};
    return one, two, three, four;    
end

TestSuite {
    preorder_traversal_order = function()
        local one, two, three, four = dependency_graph();
        local expected_order = { 'one', 'two', 'three', 'four' };
        local index = 1;
        local failures = preorder( one, function( target )
            if index == 1 then
                CHECK( target == one );
            elseif index == 2 then
                CHECK( target == two );
            elseif index == 3 then
                CHECK( target == three );
            elseif index == 4 then
                CHECK( target == four );
            end
            index = index + 1;
        end );
        CHECK_EQUAL( 0, failures );
    end;

    assert_in_preorder_traversal = function()
        local one = dependency_graph();
        local failures = preorder( one, function( target ) 
            assert( false );
        end );
        CHECK_EQUAL( 1, failures );
    end;

    error_in_preorder_traversal = function()
        local one = dependency_graph();
        local failures = preorder( one, function( target ) 
            error( 'testing error in preorder traversal' );
        end );
        CHECK_EQUAL( 1, failures );
    end;

    yield_from_execute_in_preorder_traversal = function()
        local one = dependency_graph();
        local echo_dev_null = 'echo >/dev/null';
        if operating_system() == 'windows' then
            echo_dev_null = 'echo >nul';
        end
        local failures = preorder( one, function( target ) 
            shell( {echo_dev_null; target:id()} );
        end );
        CHECK_EQUAL( 0, failures );
    end;

    non_existing_executable_in_preorder_traversal = function()
        local one = dependency_graph();
        local failures = preorder( one, function( target ) 
            system( '/this/executable/does/not/exist', '' );
        end );
        CHECK_EQUAL( 1, failures );
    end;

    yield_from_buildfile_in_preorder_traversal = function()
        local one = dependency_graph();
        create( 'yield_from_buildfile_in_preorder_traversal.build', nil, [[
            buildfile( 'yield_from_recursive_buildfile_in_preorder_traversal.build' );
        ]] );
        create( 'yield_from_recursive_buildfile_in_preorder_traversal.build' );
        local failures = preorder( one, function( target )
            buildfile( 'yield_from_buildfile_in_preorder_traversal.build' );
        end );
        CHECK_EQUAL( 0, failures );
    end;

    non_existing_buildfile_in_preorder_traversal = function()
        local one = dependency_graph();
        local failures = preorder( one, function( target ) 
            assert( false );
        end );
        CHECK_EQUAL( 1, failures );
    end;
};
