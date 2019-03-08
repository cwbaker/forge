
local App = forge:TargetPrototype( 'App' );

function App.create( forge, identifier )
    local filename = forge:interpolate( identifier );
    local app = forge:Target( ('%s$$'):format(identifier), App );
    app:add_ordering_dependency( forge:Directory(filename) );
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
            forge:system( xcrun, ('xcrun dsymutil -o "%s.dSYM" "%s"'):format(target:ordering_dependency(), executable) );
            if target.settings.strip then 
                forge:system( xcrun, ('xcrun strip "%s"'):format(executable) );
            end
        end
    end

    local provisioning_profile = _G.provisioning_profile or target.provisioning_profile;
    if provisioning_profile then
        local embedded_provisioning_profile = ("%s/embedded.mobileprovision"):format( target:ordering_dependency() );
        forge:rm( embedded_provisioning_profile );
        forge:cp( embedded_provisioning_profile, provisioning_profile );
    end

    local command_line = {
        "codesign";
        ('-s "%s"'):format( _G.signing_identity or settings.xcode.signing_identity );
        "--force";
        "--no-strict";
        "-vv";
        ('"%s"'):format( target:ordering_dependency() );
    };
    local entitlements = target.entitlements;
    if entitlements then 
        table.insert( command_line, ('--entitlements "%s"'):format(entitlements) );
    end

    local codesign = settings.xcode.codesign;
    forge:system( codesign, table.concat(command_line, ' '), environment );
end

return App;
