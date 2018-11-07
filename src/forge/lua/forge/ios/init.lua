
local ios = {};

function ios.configure( settings )
    local function autodetect_iphoneos_sdk_version()
        local sdk_version = "";
        local sdk_build_version = "";

        local xcodebuild = "/usr/bin/xcodebuild";
        local arguments = "xcodebuild -sdk iphoneos -version";
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

    local local_settings = forge.local_settings;
    if not local_settings.ios then
        local sdk_version, sdk_build_version = autodetect_iphoneos_sdk_version();
        local xcode_version, xcode_build_version = autodetect_xcode_version();
        local os_version = autodetect_macos_version();
        local_settings.updated = true;
        local_settings.ios = {
            xcrun = "/usr/bin/xcrun";
            signing_identity = "iPhone Developer";
            codesign = "/usr/bin/codesign";
            plutil = "/usr/bin/plutil";
            sdk_name = "iphoneos";
            sdk_version = sdk_version;
            sdk_build_version = sdk_build_version;
            xcode_version = xcode_version;
            xcode_build_version = xcode_build_version;
            os_version = os_version;
            architectures = { "armv7", "arm64" };
        };
    end
end

function ios.initialize( settings )
    if forge:operating_system() == 'macos' then 
        for _, architecture in ipairs(settings.ios.architectures) do 
            local xcode_clang_forge = forge:configure {
                obj = ('%s/cc_ios_%s'):format( settings.obj, architecture );
                platform = 'ios';
                sdkroot = 'iphoneos';
                xcrun = settings.ios.xcrun;
                architecture = architecture;
                default_architecture = architecture;
                ios_deployment_target = '8.0';
                targeted_device_family = '1,2';
                provisioning_profile = forge:home( 'sweet/sweet_software/dev.mobileprovision' );
                obj_directory = ios.obj_directory;
            };
            local xcode_clang = require 'forge.xcode_clang';
            xcode_clang.register( xcode_clang_forge );
            forge:add_default_build( ('cc_ios_%s'):format(architecture), xcode_clang_forge );
        end
    end
end

function ios.obj_directory( forge, target )
    local relative_path = forge:relative( target:working_directory():path(), forge:root() );
    return forge:absolute( relative_path, forge.settings.obj );
end

-- Find the first iOS .app bundle found in the dependencies of the
-- passed in directory.
function ios.find_app( directory )
    local directory = directory or forge:find_target( forge:initial("all") );
    for _, dependency in directory:dependencies() do
        if dependency:prototype() == ios.App then 
            return dependency;
        end
    end
end

-- Deploy the fist iOS .app bundle found in the dependencies of the current
-- working directory.
function ios.deploy( app )
    local ios_deploy = forge.settings.ios.ios_deploy;
    if ios_deploy then 
        assertf( app, "No ios.App target to deploy" );
        assertf( forge:is_file(ios_deploy), "No 'ios-deploy' executable found at '%s'", ios_deploy );
        forge:system( ios_deploy, ('ios-deploy --timeout 1 --bundle "%s"'):format(app:filename()) );
    else
        printf( "No 'ios-deploy' executable specified by 'settings.ios.ios_deploy'" );
    end
end

require 'forge.ios.App';

forge:register_module( ios );
forge.ios = ios;
return ios;
