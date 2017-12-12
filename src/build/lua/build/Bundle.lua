
BundlePrototype = TargetPrototype { "Bundle" };

function BundlePrototype.build( bundle )
    if platform == "ios" then
        if bundle:is_outdated() then
            local xcrun = bundle.settings.ios.xcrun;
            if bundle.settings.generate_dsym_bundle then 
                local executable;
                for dependency in bundle:get_dependencies() do 
                    if dependency:prototype() == LipoPrototype then 
                        executable = dependency:get_filename();
                        break;
                    end
                end
                if executable then 
                    build.system( xcrun, [[xcrun dsymutil -o "%s.dSYM" "%s"]] % {bundle:get_filename(), executable} );
                    if bundle.settings.strip then 
                        build.system( xcrun, [[xcrun strip "%s"]] % executable );
                    end
                end
            end

            local provisioning_profile = _G.provisioning_profile or bundle.settings.provisioning_profile;
            if provisioning_profile then
                local embedded_provisioning_profile = "%s/embedded.mobileprovision" % {bundle:get_filename()};
                rm( embedded_provisioning_profile );
                cp( provisioning_profile, embedded_provisioning_profile );
            end

            local command_line = {
                'xcrun --sdk %s codesign' % ios.sdkroot_by_target_and_platform( bundle, platform );
                '-s "%s"' % {_G.signing_identity or bundle.settings.ios.signing_identity};
                '--force';
                '--no-strict';
                '-vv';
                '"%s"' % bundle:get_filename();
            };
            local entitlements = bundle.entitlements;
            if entitlements then 
                table.insert( command_line, [[--entitlements "%s"]] % entitlements );
            end

            local resource_rules = bundle.resource_rules;
            if resource_rules then 
                table.insert( command_line, [[--resource-rules "%s"]] % resource_rules );
            end

            putenv( "CODESIGN_ALLOCATE", bundle.settings.ios.codesign_allocate );
            build.system( xcrun, table.concat(command_line, " ") );

            local ios_deploy = bundle.settings.ios.ios_deploy;
            if ios_deploy then 
                build.system( ios_deploy, [[ios-deploy -b "%s"]] % bundle:get_filename() );
            end
        end
    end
end

function BundlePrototype.clean( bundle )
    if platform == "ios" or platform == "ios_simulator" then
        rmdir( bundle:get_filename() );
    end
end

function Bundle( id )
    build.begin_target();
    return function( dependencies )
        local result = build.end_target( function()
            local bundle;
            local settings = build.push_settings( dependencies.settings or {} );
            if build.built_for_platform_and_variant(settings) then
                bundle = target( id, BundlePrototype, {settings = dependencies.settings} );
                build.push_settings {
                    bin = "%s/%s.app" % { settings.bin, id };
                    data = "%s/%s.app" % { settings.bin, id };
                };

                if platform == "ios" then
                    local entitlements = dependencies.entitlements;
                    if entitlements then 
                        bundle.entitlements = "%s/%s" % {obj_directory(bundle), "Entitlements.plist"};
                        table.insert( dependencies, Generate (bundle.entitlements) (entitlements) );
                    end

                    local resource_rules = dependencies.resource_rules;
                    if resource_rules then 
                        bundle.resource_rules = "%s/%s.app/ResourceRules.plist" % { settings.bin, id };
                        table.insert( dependencies, Copy (bundle.resource_rules) (resource_rules) );
                    end
                end

                build.add_package_dependencies( bundle, "%s/%s.app" % {settings.bin, id}, dependencies );
                build.pop_settings();
            end
            build.pop_settings();
            return bundle;
        end );
        return result;
    end
end
