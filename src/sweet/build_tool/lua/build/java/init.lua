
java = {};

function java.configure( settings )
    local function autodetect_jdk_directory()
        if build:operating_system() == "windows" then
            return "C:/Program Files/Java/jdk1.6.0_39";
        else
            return "/Library/Java/Home";
        end
    end

    local function autodetect_ivy()
        if build:operating_system() == 'windows' then 
            return 'C:/Program Files/Apache Ivy/ivy-2.5.0-rc1.jar';
        else
            return build:home( 'apache-ivy-2.5.0-rc1/ivy-2.5.0-rc1.jar' );
        end
    end

    local function autodetect_ivy_cache_directory() 
        return build:home( '.ivy2/cache' );
    end

    local function autodetect_unzip() 
        if build:operating_system() == 'windows' then 
            return 'unzip.exe';
        else 
            return '/usr/bin/unzip';
        end
    end

    local local_settings = build.local_settings;
    if not local_settings.java then
        local_settings.updated = true;
        local_settings.java = {
            jdk_directory = autodetect_jdk_directory();
            ivy = autodetect_ivy();
            ivy_cache_directory = autodetect_ivy_cache_directory();
            unzip = autodetect_unzip();
        };
    end
end

function java.initialize( settings )
    local settings = build.settings;
    settings.gen_directory = java.gen_directory;
    settings.classes_directory = java.classes_directory;
end

function java.add_jar_dependencies( jar, jars )
    if jars and platform ~= "" then
        for _, value in ipairs(jars) do
            jar:add_dependency( build:target(build:root(value)) );
        end
    end
end

function java.gen_directory( target )
    return string.format( "%s/%s", target.settings.gen, build:relative(target:working_directory():path(), build:root()) );
end

function java.classes_directory( target )
    return string.format( "%s/%s", target.settings.classes, build:relative(target:working_directory():path(), build:root()) );
end

require "build.java.Ivy";
require "build.java.Jar";
require "build.java.Java";

build:register_module( java );
