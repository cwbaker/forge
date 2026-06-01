
-- Tests for the rule constructors documented in
-- docs/reference/rule-functions.md: Rule, FileRule, PatternRule, and
-- GroupRule.  Calling a rule prototype dispatches to its create() function as
-- `rule.create(toolset, identifier, rule)`.
--
-- Note: rule-functions.md also documents ToolsetRule() but it is not
-- implemented; toolsets are constructed through forge.Toolset().

-- A fresh toolset per test - the unit test fixture clears the graph between
-- tests, so the toolset is rebuilt each time rather than shared.
local function make_toolset()
    local forge = require( 'forge' ):load();
    return forge.Toolset() {};
end

SUITE 'rule-function-tests';

-- Rule()

TEST_FIXTURE(_unit_test_setup, 'rule_create_override_is_invoked', function()
    local toolset = make_toolset();
    local Custom = Rule( 'Custom' );
    local created_identifier = nil;
    function Custom.create( toolset, identifier, rule )
        created_identifier = identifier;
        return toolset:Target( identifier, rule );
    end
    local target = Custom( toolset, 'custom_target' );
    CHECK_EQUAL( 'custom_target', created_identifier );
    CHECK( target ~= nil );
    CHECK( target:rule() == Custom );
end);

-- FileRule()

TEST_FIXTURE(_unit_test_setup, 'file_rule_sets_filename_and_adds_directory_ordering_dependency', function()
    local toolset = make_toolset();
    local Generated = FileRule( 'Generated' );
    local target = Generated( toolset, 'file_rule_dir/generated.txt' );
    CHECK( target ~= nil );
    CHECK( target:cleanable() );
    CHECK_EQUAL( target:path(), target:filename() );
    local found_directory = false;
    local index = 1;
    local dependency = target:ordering_dependency( index );
    while dependency do
        if leaf(dependency:path()) == 'file_rule_dir' then
            found_directory = true;
        end
        index = index + 1;
        dependency = target:ordering_dependency( index );
    end
    CHECK( found_directory );
end);

-- PatternRule()

TEST_FIXTURE(_unit_test_setup, 'pattern_rule_creates_one_output_per_input', function()
    local toolset = make_toolset();
    local Patterned = PatternRule( 'Patterned' );
    local outputs = Patterned( toolset, 'patterned/%1' ) { 'pattern_a.txt'; 'pattern_b.txt' };
    CHECK_EQUAL( 2, #outputs );
    for _, output in ipairs(outputs) do
        CHECK( output:cleanable() );
        -- each output depends on its corresponding source file
        CHECK( output:dependency(1) ~= nil );
    end
end);

-- GroupRule()

TEST_FIXTURE(_unit_test_setup, 'group_rule_collects_inputs_under_one_group_target', function()
    local toolset = make_toolset();
    local Grouped = GroupRule( 'Grouped' );
    local group = Grouped( toolset, 'grouped/%1' ) { 'group_a.txt'; 'group_b.txt' };
    local group_target = group[1];
    CHECK( group_target ~= nil );
    CHECK( group_target:dependency(1) ~= nil );
    CHECK( group_target:dependency(2) ~= nil );
    CHECK( group_target:dependency(3) == nil );
end);
