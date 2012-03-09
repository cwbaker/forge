
local libraries = libraries or 50;
local classes = classes or 100;
local internal_includes = internal_includes or 15;
local external_includes = external_includes or 5;

local build = "d:/usr/local/bin/build.exe";
local make = "d:/usr/local/bin/make.exe";
local scons = "c:/python26/scripts/scons.bat";
    
dofile( root("generate_source_files.lua") );
dofile( root("generate_build_files.lua") );
dofile( root("generate_make_files.lua") );
dofile( root("generate_scons_files.lua") );

function test( directory, command, arguments )
    local start = ticks();
    pushd( directory );
    if execute(command, command.." "..arguments) ~= 0 then
        error( arguments.." failed" );
    end
    popd();
    local finish = ticks();    
    return finish - start;
end

rmdir( "make" );
generate_source_files( root("make"), libraries, classes, internal_includes, external_includes );
generate_make_files( libraries, classes );
local make_full_build = test( "make", make, "" );
local make_incremental_build = test( "make", make, "" );
local make_single_library_incremental_build = test( "make", make, "lib_"..tostring(libraries)..".build" );

rmdir( "scons" );
generate_source_files( root("scons"), libraries, classes, internal_includes, external_includes );
generate_scons_files( libraries, classes );
local scons_full_build = test( "scons", scons, "" );
local scons_incremental_build = test( "scons", scons, "" );
local scons_single_library_incremental_build = test( "scons", scons, "lib_"..tostring(libraries) );

rmdir( "build" );
generate_source_files( root("build"), libraries, classes, internal_includes, external_includes );
generate_build_files( libraries, classes );
local build_full_build = test( "build", build, "-s " );
local build_incremental_build = test( "build", build, "" );
local build_single_library_incremental_build = test( "build", build, "target=lib_"..tostring(libraries) );

print( "" );
print( "tool, full, incremental, single_incremental" );
print( string.format("make, %d, %d, %d", make_full_build, make_incremental_build, make_single_library_incremental_build) );
print( string.format("scons, %d, %d, %d", scons_full_build, scons_incremental_build, scons_single_library_incremental_build) );
print( string.format("build, %d, %d, %d", build_full_build, build_incremental_build, build_single_library_incremental_build) );
