
function boost_library( name )
    local prefix, toolset, runtime, extension;
    if platform == "msvc" then
        prefix = "lib";
        toolset = "vc90";
        extension = ".lib";

        if settings.runtime_library == "static" then
            runtime = "s";
        elseif settings.runtime_library == "static_debug" then
            runtime = "sgd";
        elseif settings.runtime_library == "dynamic" then
            runtime = "";
        elseif settings.runtime_library == "dynamic_debug" then
            runtime = "gd";
        end
    elseif platform == "mingw" then
        prefix = "";
        toolset = "mgw46";
        extension = "";

        if settings.runtime_library == "static" then
            runtime = "s";
        elseif settings.runtime_library == "static_debug" then
            runtime = "sd";
        elseif settings.runtime_library == "dynamic" then
            runtime = "";
        elseif settings.runtime_library == "dynamic_debug" then
            runtime = "d";
        end      
    end

    local version = string.gsub( settings.boost.version, "%.", "_" );
    return "%s%s-%s-mt-%s-%s%s" % { prefix, name, toolset, runtime, version, extension };    
end

