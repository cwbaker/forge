
AndroidResourcePrototype = TargetPrototype { "AndroidResource" };

function AndroidResourcePrototype.generate( resource )
    local build_tools_directory = resource.settings.android.build_tools_directory;
    local sdk_directory = resource.settings.android.sdk_directory;
    local sdk_platform = resource.settings.android.sdk_platform;
    local aapt = "%s/aapt" % build_tools_directory;
    local output_directory = "gen";
    local extra_packages = "";
    if resource.extra_packages then 
        extra_packages = "--extra-packages %s" % table.concat( resource.extra_packages, ":" );
    end
    local resources = table.concat( resource.settings.resources, " -S " );
    local android_jar = "%s/platforms/%s/android.jar" % { sdk_directory, sdk_platform };
    build.system( aapt, [[aapt package --auto-add-overlay -f -m -J "%s" -S %s %s -M AndroidManifest.xml -I "%s"]] % {output_directory, resources, extra_packages, android_jar} );
end

function AndroidResourcePrototype.static_depend( resource )
    local filename = resource:get_filename();
    if not exists(filename) then
        local directory = branch( filename );
        if not exists(directory) then 
            mkdir( directory );
        end
        resource:generate();
    end
end

function AndroidResourcePrototype.build( resource )
    if resource:is_outdated() then
        resource:generate();
    end
end

function AndroidResourcePrototype.clean( resource )
    rm( resource:get_filename() );
end

function AndroidResource( filename )
    build.begin_target();
    return function ( definition )
        return build.end_target( function()
            local resource;
            local settings = build.current_settings();
            if build.built_for_platform_and_variant(settings) then
                resource = file( filename, AndroidResourcePrototype, definition );
                resource.settings = settings;
                resource:add_dependency( Directory(branch(resource:get_filename())) );
            end    
            return { resource };
        end );
    end
end
