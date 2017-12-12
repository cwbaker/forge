
PackagePrototype = TargetPrototype { "Package" };

function PackagePrototype.build( package )
    if package:is_outdated() and platform == "android" then
        local aapt = "%s/platform-tools/aapt.exe" % package.settings.android.sdk_directory;
        local resources = table.concat( build.expand(package.resources), " -S " );
        local android_jar = "%s/platforms/android-16/android.jar" % package.settings.android.sdk_directory;
        build.system( aapt, [[aapt package -f -M AndroidManifest.xml -S %s -I "%s" -F "%s.unaligned"]] % {resources, android_jar, package:get_filename()} );

        pushd( "%s/%s" % {branch(package:get_filename()), package:id()} );
        for file in find("") do 
            if is_file(file) then
                build.system( aapt, [[aapt add -f "%s.unaligned" "%s"]] % {relative(package:get_filename()), relative(file)} );
            end
        end

        local jarsigner = "%s/bin/jarsigner.exe" % package.settings.android.jdk_directory;
        local keystore = relative( "%s/debug.keystore" % package:get_working_directory():path() );
        build.system( jarsigner, "jarsigner -keystore %s -storepass android %s.unaligned androiddebugkey" % {keystore, relative(package:get_filename())} );

        local zipalign = "%s/tools/zipalign.exe" % package.settings.android.sdk_directory;
        build.system( zipalign, "zipalign -f 4 %s.unaligned %s" % {relative(package:get_filename()), relative(package:get_filename())} );
        popd();
    end
end

function PackagePrototype.clean( package )
    if platform == "android" then
        rm( "%s.unaligned" % package:get_filename() );
        rm( package:get_filename() );
    end
end

function Package( id )
    build.begin_target();
    return function( dependencies )
        local result = build.end_target( function()
            local package;
            local settings = build.push_settings( dependencies.settings or {} );
            if build.built_for_platform_and_variant(settings) then
                package = target( id, PackagePrototype, {resources = dependencies.resources} );
                build.push_settings {
                    bin = "%s/%s" % { settings.bin, id };
                    data = "%s/%s" % { settings.bin, id };
                };
                build.add_package_dependencies( package, "%s/%s.apk" % {settings.bin, id}, dependencies );
                build.pop_settings();
            end
            build.pop_settings();
            return package;
        end);
        return result;
    end
end
