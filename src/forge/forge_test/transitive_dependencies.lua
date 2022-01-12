

local executions = {};

local function clear_executions()
    executions = {};
end

local function print_executions()
    for _, execution in ipairs(executions) do
        printf( '%s', execution );
    end
    printf( '\n\n' );
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
                    toolset:Cc 'baz.o' {
                        'baz.c';
                    };
                };
                toolset:Cc 'bar.o' {
                    'bar.c';
                };
            };
            toolset:Cc 'foo.o' {
                'foo.c';
            };
        };
        toolset:Cc 'exe.o' {
            'exe.c';
        };
    };
    clear_executions();
    _G.execute = execute;
    _G.printf = printf;
    _G.find_initial_target = function() return exe end;
    _G.goal = tostring( exe );
    build();
    forge:save();
    return exe;
end

remove( absolute('.forge') );

if operating_system() == 'linux' then
    require 'forge';
    local cc = require 'forge.cc' {};

    create( absolute('baz.o.o'), 1 );
    create( absolute('libbaz.a'), 1 );
    create( absolute('bar.c'), 1 );
    create( absolute('bar.o.o'), 1 );
    create( absolute('libbar.a'), 1 );
    create( absolute('foo.o.o'), 1 );
    create( absolute('libfoo.a'), 1 );
    create( absolute('exe.o.o'), 1 );
    create( absolute('exe'), 1 );

    local exe = dependency_graph( cc );
    CHECK( find_initial_target() == exe );
    CHECK( #executions == 8 );
    CHECK( executions[1] and executions[1]:find('gcc') );
    CHECK( executions[2] and executions[2]:find('gcc') );
    CHECK( executions[3] and executions[3]:find('ar') );
    CHECK( executions[4] and executions[4]:find('gcc') );
    CHECK( executions[5] and executions[5]:find('ar') );
    CHECK( executions[6] and executions[6]:find('ar') );
    CHECK( executions[7] and executions[7]:find('gcc') );
    CHECK( executions[8] and executions[8]:find('g++') );

    touch( absolute('bar.c'), 2 );

    local exe = dependency_graph( cc );
    CHECK( #executions == 8 );
    CHECK( executions[1] and executions[1]:find('gcc') );
    CHECK( executions[2] and executions[2]:find('gcc') );
    CHECK( executions[3] and executions[3]:find('ar') );
    CHECK( executions[4] and executions[4]:find('gcc') );
    CHECK( executions[5] and executions[5]:find('ar') );
    CHECK( executions[6] and executions[6]:find('ar') );
    CHECK( executions[7] and executions[7]:find('gcc') );
    CHECK( executions[8] and executions[8]:find('g++') );

    touch( absolute('bar.o.o'), 2 );
    touch( absolute('baz.o.o'), 2 );
    touch( absolute('foo.o.o'), 2 );
    touch( absolute('libbar.a'), 2 );
    touch( absolute('libfoo.a'), 2 );
    touch( absolute('exe'), 2 );

    local exe = dependency_graph( cc );
    CHECK( #executions == 6 );
    CHECK( executions[1] and executions[1]:find('gcc') );
    CHECK( executions[2] and executions[2]:find('gcc') );
    CHECK( executions[3] and executions[3]:find('ar') );
    CHECK( executions[4] and executions[4]:find('ar') );
    CHECK( executions[5] and executions[5]:find('gcc') );
    CHECK( executions[6] and executions[6]:find('g++') );

    remove( absolute('.forge') );
end
