
local ios = {};

-- This is here solely for reference while I'm moving to use the xcode_clang
-- module instead.
function ios.initialize__deprecated__( settings )
    if forge:operating_system() == 'macos' then 
        for _, architecture in ipairs(settings.ios.architectures) do 
            local xcode_clang_forge = forge:configure {
                obj = ('%s/cc_ios_%s'):format( settings.obj, architecture );
                platform = 'ios';
                sdkroot = 'iphoneos';
                xcrun = settings.ios.xcrun;
                architecture = architecture;
                ios_deployment_target = '8.0';
                targeted_device_family = '1,2';
                provisioning_profile = forge:home( 'sweet/sweet_software/dev.mobileprovision' );
            };
            local xcode_clang = require 'forge.xcode_clang';
            xcode_clang.register( xcode_clang_forge );
            forge:add_default_build( ('cc_ios_%s'):format(architecture), xcode_clang_forge );
        end
    end
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
