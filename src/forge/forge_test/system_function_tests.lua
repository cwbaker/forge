
-- Tests for the system functions documented in
-- docs/reference/system-functions.md.  run() and shell() are defined by the
-- forge module so it is required here; the rest are built-in globals.
--
-- execute(), run(), and shell() suspend the running coroutine until the
-- spawned process finishes.  These tests exercise that path directly, which
-- is only possible because the unit test driver runs each test through a
-- yieldable Lua xpcall() rather than a non-yieldable C call.

require 'forge';

-- Return the (command, arguments) pair that runs `script` through the host
-- system's native shell, so process tests stay portable across platforms.
local function shell_command( script )
    if operating_system() == 'windows' then
        return 'C:/windows/system32/cmd.exe', ('cmd /c "%s"'):format( script );
    end
    return '/bin/sh', ('sh -c "%s"'):format( script );
end

SUITE 'system-function-tests';

-- operating_system()

TEST_FIXTURE(_unit_test_setup, 'operating_system_is_a_known_value', function()
    local name = operating_system();
    CHECK( name == 'linux' or name == 'windows' or name == 'macos' );
end);

-- hash()

TEST_FIXTURE(_unit_test_setup, 'hash_is_deterministic', function()
    CHECK_EQUAL( hash{a = 1; b = 2; c = 'three'}, hash{a = 1; b = 2; c = 'three'} );
end);

TEST_FIXTURE(_unit_test_setup, 'hash_differs_for_different_content', function()
    CHECK( hash{a = 1} ~= hash{a = 2} );
end);

-- print()

TEST_FIXTURE(_unit_test_setup, 'print_runs_without_error', function()
    quiet();
    print( 'system_function_tests print output' );
    CHECK( true );
end);

-- ticks()

-- ticks() reports CPU time elapsed since Forge started, so it is monotonic
-- but is not advanced by sleep() (which consumes no CPU time).
TEST_FIXTURE(_unit_test_setup, 'ticks_is_non_negative_and_monotonic', function()
    local first = ticks();
    local second = ticks();
    CHECK( first >= 0 );
    CHECK( second >= first );
end);

-- sleep()

TEST_FIXTURE(_unit_test_setup, 'sleep_runs_without_error', function()
    sleep( 10 );
    CHECK( true );
end);

-- execute()

TEST_FIXTURE(_unit_test_setup, 'execute_returns_process_exit_code', function()
    local result = execute( shell_command('exit 7') );
    CHECK_EQUAL( 7, result );
end);

TEST_FIXTURE(_unit_test_setup, 'execute_returns_zero_for_successful_process', function()
    local result = execute( shell_command('exit 0') );
    CHECK_EQUAL( 0, result );
end);

-- run()

TEST_FIXTURE(_unit_test_setup, 'run_succeeds_for_zero_exit', function()
    local ok = pcall( run, shell_command('exit 0') );
    CHECK( ok );
end);

TEST_FIXTURE(_unit_test_setup, 'run_raises_for_non_zero_exit', function()
    local ok = pcall( run, shell_command('exit 5') );
    CHECK( not ok );
end);

-- shell()

TEST_FIXTURE(_unit_test_setup, 'shell_succeeds_for_zero_exit', function()
    local ok = pcall( shell, 'exit 0' );
    CHECK( ok );
end);

TEST_FIXTURE(_unit_test_setup, 'shell_raises_for_non_zero_exit', function()
    local ok = pcall( shell, 'exit 4' );
    CHECK( not ok );
end);

-- wait()

TEST_FIXTURE(_unit_test_setup, 'wait_runs_without_error', function()
    wait();
    CHECK( true );
end);
