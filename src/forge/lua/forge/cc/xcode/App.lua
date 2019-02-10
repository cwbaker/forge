
local App = forge:TargetPrototype( 'App' );

local function default_identifier_filename( forge, identifier, settings )
    local identifier = forge:interpolate( identifier, settings );
    local leaf = forge:leaf( identifier );
    local branch = settings.bin;
    if forge:is_absolute(identifier) then 
        branch = forge:branch( identifier );
    end
    local identifier = ("%s$$"):format( identifier );
    local filename = ("%s/%s"):format( branch, leaf );
    return identifier, filename;
end

function App.create( forge, identifier )
    local settings = forge.settings;
    local identifier, filename = default_identifier_filename( forge, identifier, settings );
    local app = forge:Target( identifier, App );
    app:set_filename( filename );
    app:set_cleanable( true );
    app:add_ordering_dependency( forge:Directory(forge:branch(app)) );
    return app;
end

function App.depend( forge, target, dependencies )
    local settings = forge.settings;
    local entitlements = dependencies.entitlements;
    if entitlements then 
        table.insert( dependencies, entitlements );
        target.entitlements = entitlements:filename();
        dependencies.entitlements = nil;
    end
    return forge.Target.depend( forge, target, dependencies );
end

function App.build( forge, target )
    local settings = forge.settings;
    local xcrun = settings.xcode.xcrun;
    if target.settings.generate_dsym_bundle then 
        local executable;
        for _, dependency in target:dependencies() do 
            if dependency:prototype() == forge.Lipo then 
                executable = dependency:filename();
                break;
            end
        end
        if executable then 
            forge:system( xcrun, ('xcrun dsymutil -o "%s.dSYM" "%s"'):format(target:filename(), executable) );
            if target.settings.strip then 
                forge:system( xcrun, ('xcrun strip "%s"'):format(executable) );
            end
        end
    end

    local provisioning_profile = _G.provisioning_profile or target.provisioning_profile;
    if provisioning_profile then
        local embedded_provisioning_profile = ("%s/embedded.mobileprovision"):format( target:filename() );
        forge:rm( embedded_provisioning_profile );
        forge:cp( embedded_provisioning_profile, provisioning_profile );
    end

    local command_line = {
        "codesign";
        ('-s "%s"'):format( _G.signing_identity or settings.xcode.signing_identity );
        "--force";
        "--no-strict";
        "-vv";
        ('"%s"'):format( target );
    };
    local entitlements = target.entitlements;
    if entitlements then 
        table.insert( command_line, ('--entitlements "%s"'):format(entitlements) );
    end

    local codesign = settings.xcode.codesign;
    forge:system( codesign, table.concat(command_line, ' '), environment );
end

function App.clean( forge, target )
    forge:rmdir( target:filename() );
end

return App;
