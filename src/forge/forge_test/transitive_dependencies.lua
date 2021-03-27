

local executions = {};

local function clear_executions()
    executions = {};
end

local function print_executions()
    for _, execution in ipairs(executions) do
        printf( '%s', execution );
    end
end

local function execute( command, command_line )
    table.insert( executions, ('%s %s'):format(command, command_line) );
    return 0;
end

local function printf()
end

local function dependency_graph( toolset )
    forge:reload();
    local exe = toolset:Executable 'exe' {
        toolset:StaticLibrary 'foo' {
            'baz';
            toolset:StaticLibrary 'bar' {
                toolset:StaticLibrary 'baz' {
                    'baz.o';
                };
                toolset:Cc 'bar' {
                    'bar.c';
                };
            };
            'foo.o';
        };
        'exe.o';
    };
    clear_executions();
    _G.execute = execute;
    _G.printf = printf;
    _G.goal = exe;
    build();
    forge:save();
    return exe;
end

remove( absolute('.forge') );

if operating_system() ~= 'windows' then
    require 'forge';
    local cc = require 'forge.cc.gcc' {};

    create( absolute('baz.o'), 1 );
    create( absolute('libbaz.a'), 1 );
    create( absolute('bar.c'), 1 );
    create( absolute('bar.o'), 1 );
    create( absolute('libbar.a'), 1 );
    create( absolute('foo.o'), 1 );
    create( absolute('libfoo.a'), 1 );
    create( absolute('exe.o'), 1 );
    create( absolute('exe'), 1 );

    local exe = dependency_graph( cc );
    local libraries = exe:find_transitive_libraries();
    CHECK( libraries[1] == find_target('foo') );
    CHECK( libraries[2] == find_target('bar') );
    CHECK( libraries[3] == find_target('baz') );
    CHECK( find_initial_target(exe) == exe );
    CHECK( #executions == 5 );
    CHECK( executions[1]:find('ar') );
    CHECK( executions[2]:find('gcc') );
    CHECK( executions[3]:find('ar') );
    CHECK( executions[4]:find('ar') );
    CHECK( executions[5]:find('g++') );

    touch( absolute('bar.c'), 2 );

    local exe = dependency_graph( cc );
    CHECK( #executions == 3 );
    CHECK( executions[1]:find('gcc') );
    CHECK( executions[2]:find('ar') );
    CHECK( executions[3]:find('g++') );

    touch( absolute('bar.o'), 2 );
    touch( absolute('libbar.a'), 2 );
    touch( absolute('libfoo.a'), 2 );
    touch( absolute('exe'), 2 );

    local exe = dependency_graph( cc );
    CHECK( #executions == 0 );

    remove( absolute('.forge') );
end
