
# C/C++ Library

```
buildfile "build/build.build";
buildfile "build_tool_test/build_tool_test.build";

local settings = build.current_settings();
for _, architecture in ipairs(settings.architectures) do
    build.Library ("build_tool", architecture) {
        libraries = {
            "boost/libs/filesystem/src/boost_filesystem",
            "boost/libs/system/src/boost_system",
            "sweet/assert/assert",
            "sweet/error/error",
            "sweet/lua/lua",
            "lua/liblua",
            "sweet/path/path",
            "sweet/persist/persist",
            "sweet/pointer/pointer",
            "sweet/process/process",
            "sweet/rtti/rtti",
            "sweet/thread/thread"
        };

        Cxx (architecture) {
            -- Disable automatic linking to Boost libraries.
            defines = { "BOOST_ALL_NO_LIB" };
            "BuildTool.cpp",
            "BuildToolEventSink.cpp",
            "Error.cpp", 
            "Executor.cpp",
            "Graph.cpp",
            "Job.cpp",
            "OsInterface.cpp",
            "Pattern.cpp", 
            "Scanner.cpp",
            "Scheduler.cpp", 
            "Arguments.cpp",
            "ScriptInterface.cpp",
            "Target.cpp",
            "TargetPrototype.cpp",
            "Environment.cpp"
        };
    }
end
```
