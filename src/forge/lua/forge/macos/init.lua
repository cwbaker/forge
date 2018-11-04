
local macos = {};

function macos.configure( settings )
    local function autodetect_macos_sdk_version()
        local sdk_version = "";
        local sdk_build_version = "";

        local xcodebuild = "/usr/bin/xcodebuild";
        local arguments = "xcodebuild -sdk macosx -version";
        local result = forge:execute( xcodebuild, arguments, nil, nil, function(line)
            local key, value = line:match( "(%w+): ([^\n]+)" );
            if key and value then 
                if key == "ProductBuildVersion" then 
                    sdk_build_version = value;
                elseif key == "SDKVersion" then
                    sdk_version = value;
                end
            end
        end );
        assert( result == 0, "Running xcodebuild to extract SDK name and version failed" );

        return sdk_version, sdk_build_version;
    end

    local function autodetect_xcode_version()
        local xcode_version = "";
        local xcode_build_version = "";

        local xcodebuild = "/usr/bin/xcodebuild";
        local arguments = "xcodebuild -version";
        local result = forge:execute( xcodebuild, arguments, nil, nil, function(line)
            local major, minor = line:match( "Xcode (%d+)%.(%d+)" );
            if major and minor then 
                xcode_version = ("%02d%02d"):format( tonumber(major), tonumber(minor) );
            end

            local build_version = line:match( "Build version (%w+)" )
            if build_version then
                xcode_build_version = build_version;
            end
        end );
        assert( result == 0, "Running xcodebuild to extract Xcode version failed" );
        
        return xcode_version, xcode_build_version;
    end

    local function autodetect_macos_version()
        local os_version = "";

        local sw_vers = "/usr/bin/sw_vers";
        local arguments = "sw_vers -buildVersion";
        local result = forge:execute( sw_vers, arguments, nil, nil, function(line)
            local version = line:match( "%w+" );
            if version then 
                os_version = version;
            end
        end );
        assert( result == 0, "Running sw_vers to extract operating system version failed" );

        return os_version;
    end

    if forge:operating_system() == "macos" then
        local local_settings = forge.local_settings;
        if not local_settings.macos then
            local_settings.updated = true;
            local_settings.macos = {
                xcrun = "/usr/bin/xcrun";
                sdk_version = sdk_version;
                sdk_build_version = sdk_build_version;
                xcode_version = xcode_version;
                xcode_build_version = xcode_build_version;
                os_version = os_version;
            };
        end
    end
end

function macos.initialize( settings )
    if forge:operating_system() == 'macos' then
        for _, architecture in ipairs(settings.architectures) do 
            local xcode_clang_forge = forge:configure {
                obj = ('%s/cc_macos_%s'):format( settings.obj, architecture );
                platform = 'macos';
                sdkroot = 'macosx';
                xcrun = settings.macos.xcrun;
                architecture = architecture;
                default_architecture = architecture;
                obj_directory = macos.obj_directory;
            };
            local xcode_clang = require 'forge.xcode_clang';
            xcode_clang.register( xcode_clang_forge );
            forge:add_default_build( ('cc_macos_%s'):format(architecture), xcode_clang_forge );
        end

        local settings = forge.settings;
        local default_architecture = settings.default_architecture;
        local default_forge = forge:default_build( ('cc_macos_%s'):format(default_architecture) )
        assert( default_forge, 'default Forge instance not found for macOS initialization' );
        local default_settings = default_forge.settings;
        settings.obj = default_settings.obj;
        settings.platform = default_settings.platform;
        settings.sdkroot = default_settings.sdkroot;
        settings.xcrun = default_settings.xcrun;
        settings.architecture = default_settings.architecture;
        settings.default_architecture = default_settings.default_architecture;
        settings.obj_directory = default_settings.obj_directory;
        local xcode_clang = require 'forge.xcode_clang';
        xcode_clang.register( forge );
    end
end

function macos.obj_directory( forge, target )
    local relative_path = forge:relative( target:working_directory():path(), forge:root() );
    return forge:absolute( relative_path, forge.settings.obj );
end

forge:register_module( macos );
forge.macos = macos;
return macos;
