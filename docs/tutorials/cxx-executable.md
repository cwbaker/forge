
# C/C++ Executable

```
local settings = build.push_settings {
    subsystem = "CONSOLE"; 
    stack_size = 32768; 
    architectures = { "x86_64" };
    -- Disable automatic linking to Boost libraries.
    defines = { "BOOST_ALL_NO_LIB" };
};

for _, architecture in ipairs(settings.architectures) do
    build.Executable ("build", architecture) {
        libraries = {
            "boost/libs/filesystem/src/boost_filesystem",
            "boost/libs/system/src/boost_system",
            "sweet/build_tool/build_tool",
            "sweet/cmdline/cmdline",
            "sweet/lua/lua",
            "lua/liblua",
            "sweet/process/process",
            "sweet/rtti/rtti",
            "sweet/thread/thread",
            "sweet/persist/persist",
            "sweet/path/path",
            "sweet/pointer/pointer",
            "sweet/error/error",
            "sweet/assert/assert"
        };
        
        Cxx (architecture) {
            "Application.cpp", 
            "main.cpp"
        };    
    }
end

build.pop_settings();
```
