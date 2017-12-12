
local App = build.TargetPrototype( "ios.App" );

function App.create( settings, id )
    local app = build.Target( ("%s.app"):format(id), App );
    app:set_filename( ("%s/%s.app"):format(settings.bin, id) );
    app.settings = settings;
    build.push_settings {
        bin = app:filename();
        data = app:filename();
    };
    return app;
end

function App.call( app, definition )
    local entitlements = definition.entitlements;
    if entitlements then 
        app.entitlements = ("%s/%s"):format( obj_directory(app), "Entitlements.plist" );
        table.insert( definition, build.Generate (app.entitlements) (entitlements) );
    end

    for _, dependency in ipairs(definition) do 
        app:add_dependency( dependency );
    end
end

function App.build( app )
    if app:outdated() then
        local xcrun = app.settings.ios.xcrun;
        if app.settings.generate_dsym_bundle then 
            local executable;
            for _, dependency in app:dependencies() do 
                if dependency:prototype() == build.xcode.Lipo then 
                    executable = dependency:filename();
                    break;
                end
            end
            if executable then 
                build.system( xcrun, ('xcrun dsymutil -o "%s.dSYM" "%s"'):format(app:filename(), executable) );
                if app.settings.strip then 
                    build.system( xcrun, ('xcrun strip "%s"'):format(executable) );
                end
            end
        end

        local provisioning_profile = _G.provisioning_profile or app.settings.provisioning_profile;
        if provisioning_profile then
            local embedded_provisioning_profile = ("%s/embedded.mobileprovision"):format( app:filename() );
            build.rm( embedded_provisioning_profile );
            build.cp( embedded_provisioning_profile, provisioning_profile );
        end

        if platform == "ios" then
            local command_line = {
                "codesign";
                ('-s "%s"'):format( _G.signing_identity or app.settings.ios.signing_identity );
                "--force";
                "--no-strict";
                "-vv";
                ('"%s"'):format( app );
            };
            local entitlements = app.entitlements;
            if entitlements then 
                table.insert( command_line, ('--entitlements "%s"'):format(entitlements) );
            end

            local codesign = app.settings.ios.codesign;
            build.system( codesign, table.concat(command_line, " "), environment );
        end
    end
end

function App.clean( app )
    build.rmdir( app:filename() );
end

ios.App = App;
