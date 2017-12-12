
AndroidResourcePrototype = TargetPrototype { "AndroidResource" };

function AndroidResourcePrototype.build( android_resource )
    if android_resource:is_outdated() then
        local sdk_directory = android_resource.settings.android.sdk_directory;
        local sdk_platform = android_resource.settings.android.sdk_platform;
        local aapt = "%s/platform-tools/aapt" % sdk_directory;
        local output_directory = "%s/gen" % { obj_directory(android_resource) };
        local resource_directories = table.concat( android_resource, " -S " );
        local android_jar = "%s/platforms/%s/android.jar" % { sdk_directory, sdk_platform };
        local proguard_txt = "%s/proguard.txt" % obj_directory( android_resource );
        build.system( aapt, [[aapt package -f -m -J "%s" -M AndroidManifest.xml -S %s -I "%s" -G "%s"]] % {output_directory, resource_directories, android_jar, proguard_txt} );
    end
end

function AndroidResourcePrototype.clean( android_resource )
    for dependency in android_resource:get_dependencies() do
        if dependency:prototype() == nil then
            rm( dependency:path() );
        end
    end
end

function AndroidResource( resources, package )
    local android_resource;
    local settings = build.current_settings();
    if build.built_for_platform_and_variant(settings) then
        android_resource = target( "", AndroidResourcePrototype, resources );
        android_resource.settings = settings;
        android_resource:set_filename( "%s/gen/%s/R.java" % {obj_directory(android_resource), package} );
        android_resource:add_dependency( Directory(branch(android_resource:get_filename())) );
    end    
    return android_resource;
end
