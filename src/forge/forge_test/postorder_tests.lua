
TestSuite {
    error_from_lua_in_postorder_visit_is_reported_and_handled = function()
        local ErrorInPostorderVisit = Rule( 'ErrorInPostorderVisit' );
        local error_in_postorder_visit = Target( forge, 'error_in_postorder_visit', ErrorInPostorderVisit );
        local failures = postorder( error_in_postorder_visit, function(target) error('Error in postorder visit') end );
        CHECK_EQUAL( 2, failures );
        CHECK( error_message(0):find('Error in postorder visit', 1, true) ~= nil );
        CHECK_EQUAL( "Postorder visit of 'error_in_postorder_visit' failed", error_message(1) );
    end;

    unexpected_error_from_lua_in_postorder_visit_is_reported_and_handled = function()
        local UnexpectedErrorInPostorderVisit = Rule( 'UnexpectedErrorInPostorderVisit' );
        local unexpected_error_in_postorder_visit = Target( forge, 'unexpected_error_in_postorder_visit', UnexpectedErrorInPostorderVisit );
        local failures = postorder( unexpected_error_in_postorder_visit, function(target) foo.bar = 2; end );
        CHECK_EQUAL( 2, failures );
        CHECK( error_message(0):find("attempt to index a nil value (global 'foo')", 1, true) ~= nil );
        CHECK_EQUAL( "Postorder visit of 'unexpected_error_in_postorder_visit' failed", error_message(1) );
    end;

    recursive_postorder_is_reported_and_handled = function()
        local RecursivePostorderError = Rule( 'RecursivePostorderError' );
        local recursive_postorder_error = Target( forge, 'recursive_postorder_error', RecursivePostorderError );
        local failures = postorder( recursive_postorder_error, function(target) postorder(function(target) end, recursive_postorder_error) end );
        CHECK_EQUAL( 2, failures );
        CHECK( error_message(0):find('Postorder called from within preorder or postorder', 1, true) ~= nil );
        CHECK_EQUAL( "Postorder visit of 'recursive_postorder_error' failed", error_message(1) );
    end;

    recursive_postorder_during_postorder_is_reported_and_handled = function()
        local RecursivePostorderDuringPostorderError = Rule( 'RecursivePostorderDuringPostorderError' );
        local recursive_postorder_during_postorder_error = Target( forge, 'recursive_postorder_during_postorder_error', RecursivePostorderDuringPostorderError );
        local failures = postorder( recursive_postorder_during_postorder_error, function(target) postorder(function(target) end, recursive_postorder_during_postorder_error) end );
        CHECK_EQUAL( 2, failures );
        CHECK( error_message(0):find('Postorder called from within preorder or postorder', 1, true) ~= nil );
        CHECK_EQUAL( "Postorder visit of 'recursive_postorder_during_postorder_error' failed", error_message(1) );
    end;
};
