
local App = build.TargetPrototype( "ios.App" );

function App.create( _, id )
    local settings = build.current_settings();
    local app = build.Target( id, App );
    app:set_filename( ("%s/%s.app"):format(settings.bin, id) );
    app.settings = settings;
    build.push_settings {
        bin = ("%s/%s.app"):format( settings.bin, id );
        data = ("%s/%s.app"):format( settings.bin, id );
    };
    working_directory():add_dependency( app );
    return app;
end

function App.call( app, definition )
    if platform == "ios" then
        local entitlements = definition.entitlements;
        if entitlements then 
            app.entitlements = ("%s/%s"):format( obj_directory(app), "Entitlements.plist" );
            table.insert( definition, Generate(app.entitlements, entitlements) );
        end

        local resource_rules = definition.resource_rules;
        if resource_rules then 
            app.resource_rules = ("%s/ResourceRules.plist"):format( settings.bin, app:get_filename() );
            table.insert( definition, Copy(app.resource_rules, resource_rules) );
        end
    end

    local working_directory = working_directory();
    for _, dependency in ipairs(definition) do 
        working_directory:remove_dependency( dependency );
        app:add_dependency( dependency );
        dependency.module = app;
    end
end

function App.build( app )
    if app:is_outdated() then
        local xcrun = app.settings.ios.xcrun;
        if app.settings.generate_dsym_bundle then 
            local executable;
            for dependency in app:get_dependencies() do 
                if dependency:prototype() == LipoPrototype then 
                    executable = dependency:get_filename();
                    break;
                end
            end
            if executable then 
                build.system( xcrun, ([[xcrun dsymutil -o "%s.dSYM" "%s"]]):format(app:get_filename(), executable) );
                if app.settings.strip then 
                    build.system( xcrun, ([[xcrun strip "%s"]]):format(executable) );
                end
            end
        end

        local provisioning_profile = _G.provisioning_profile or app.settings.provisioning_profile;
        if provisioning_profile then
            local embedded_provisioning_profile = ("%s/embedded.mobileprovision"):format( app:get_filename() );
            rm( embedded_provisioning_profile );
            cp( provisioning_profile, embedded_provisioning_profile );
        end

        local command_line = {
            ("xcrun --sdk %s codesign"):format( ios.sdkroot_by_target_and_platform(app, platform) );
            ('-s "%s"'):format( _G.signing_identity or app.settings.ios.signing_identity );
            "--force";
            "--no-strict";
            "-vv";
            ('"%s"'):format( app:get_filename() );
        };
        local entitlements = app.entitlements;
        if entitlements then 
            table.insert( command_line, ('--entitlements "%s"'):format(entitlements) );
        end

        local resource_rules = app.resource_rules;
        if resource_rules then 
            table.insert( command_line, ('--resource-rules "%s"'):format(resource_rules) );
        end

        putenv( "CODESIGN_ALLOCATE", app.settings.ios.codesign_allocate );
        build.system( xcrun, table.concat(command_line, " ") );
    end
end

function App.clean( app )
    rmdir( app:get_filename() );
end

ios.App = App;
