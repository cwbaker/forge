
PackagePrototype = TargetPrototype { "Package" };

function PackagePrototype.build( package )
    if package:is_outdated() then
        local aapt = "%s/platform-tools/aapt.exe" % package.settings.android.sdk_directory;
        local resources = table.concat( build.expand(package.resources), " -S " );
        local android_jar = "%s/platforms/android-16/android.jar" % package.settings.android.sdk_directory;
        build.system( aapt, [[aapt package -f -M AndroidManifest.xml -S %s -I "%s" -F "%s.unaligned"]] % {resources, android_jar, package:get_filename()} );

        pushd( "%s/%s" % {branch(package:get_filename()), package:id()} );
        for dependency in package:get_dependencies() do 
            if dependency:prototype() ~= DirectoryPrototype then
                build.system( aapt, [[aapt add -f "%s.unaligned" "%s"]] % {package:get_filename(), relative(dependency:get_filename())} );
            end
        end

        local jarsigner = "%s/bin/jarsigner.exe" % package.settings.android.jdk_directory;
        build.system( jarsigner, "jarsigner -keystore ../sweet.debug.keystore -storepass n1vek0gre %s.unaligned sweet_debug_keystore" % package:get_filename() );

        local zipalign = "%s/tools/zipalign.exe" % package.settings.android.sdk_directory;
        build.system( zipalign, "zipalign -f 4 %s.unaligned %s" % {package:get_filename(), package:get_filename()} );
        popd();
    end
end

function PackagePrototype.clean( package )
    rm( "%s.unaligned" % package:get_filename() );
    rm( package:get_filename() );
end

function Package( id )
    build.push_settings {
        bin = "%s/%s" % { build.current_settings().bin, id }
    };
    return function( dependencies )
        build.pop_settings();
        local settings = build.current_settings();
        local package = target( id, PackagePrototype, {resources = dependencies.resources} );
        build.add_package_dependencies( package, "%s/%s.apk" % {settings.bin, id}, settings, dependencies );
        return package;
    end
end
