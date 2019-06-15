
local App = forge:TargetPrototype( 'App' );

function App.create( forge, identifier )
    local filename = forge:interpolate( identifier );
    local app = forge:Target( ('%s$$'):format(identifier), App );
    app:add_ordering_dependency( forge:Directory(filename) );
    return app;
end

function App.build( forge, target )
    local executable;
    local entitlements;
    local provisioning_profile;

    for _, dependency in target:dependencies() do
        local identifier = dependency:id();
        local prototype = dependency:prototype();
        if prototype == forge.Lipo then 
            executable = dependency:filename();
        elseif identifier == 'Entitlements.xml' then 
            entitlements = dependency;
        elseif extension(identifier) == '.mobileprovision' then
            provisioning_profile = _G.provisioning_profile or dependency;
        end
    end

    local settings = forge.settings;
    if settings.generate_dsym_bundle and executable then 
        local xcrun = settings.xcode.xcrun;
        system( xcrun, ('xcrun dsymutil -o "%s.dSYM" "%s"'):format(target:ordering_dependency(), executable) );
        if target.settings.strip then 
            system( xcrun, ('xcrun strip "%s"'):format(executable) );
        end
    end

    if provisioning_profile then
        local embedded_provisioning_profile = ("%s/embedded.mobileprovision"):format( target:ordering_dependency() );
        rm( embedded_provisioning_profile );
        cp( embedded_provisioning_profile, provisioning_profile );
    end

    local command_line = {
        "codesign";
        ('-s "%s"'):format( _G.signing_identity or settings.xcode.signing_identity );
        "--force";
        "--no-strict";
        "-vv";
        ('"%s"'):format( target:ordering_dependency() );
    };

    if entitlements then 
        table.insert( command_line, ('--entitlements "%s"'):format(entitlements) );
    end

    local codesign = settings.xcode.codesign;
    system( codesign, table.concat(command_line, ' '), environment );
end

return App;
