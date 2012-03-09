
local sconstruct_source = [[
env = Environment(TARGET_ARCH = 'x86')
%s
]];

local sconscript_source = [[
Import('env')
list = Split("""
%s
""")
env.StaticLibrary("lib_%d", list)
]];
        
function generate_scons_files( libraries, classes )
    local function class_list( classes )
        local class_list = ""; 
        for i = 1, classes do
            class_list = class_list..string.format( "class_%d.cpp\n", i );
        end
        return class_list;
    end

    local function library_list( libraries )
        local library_list = "";
        for i = 1, libraries do
            library_list = library_list..string.format( "env.SConscript('lib_%d/SConscript', exports=['env'])\n", i );
        end
        return library_list;
    end

    local sconstruct = io.open( root("scons/Sconstruct"), "wb" );
    assert( sconstruct, "Opening '"..root("scons/Sconstruct").."' failed" );
    sconstruct:write( string.format(sconstruct_source, library_list(libraries)) );
    sconstruct:close();
    sconstruct = nil;

    for i = 1, libraries do
        local sconscript = io.open( root("scons/lib_"..tostring(i).."/SConscript"), "wb" );
        assert( sconscript, "Opening '"..root("scons/lib_"..tostring(i).."/SConscript").."' failed" );
        sconscript:write( string.format(sconscript_source, class_list(classes), i) );
        sconscript:close();
        sconscript = nil;
    end
end
