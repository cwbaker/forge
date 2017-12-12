
local Apk = build.TargetPrototype( "android.Apk" );

function Apk.create( settings, id )
    local package = build.Target( ("%s.apk"):format(id), Apk );
    package:set_filename( ("%s/%s.apk"):format(settings.bin, id) );
    package.settings = settings;
    build.default_target( package );
    build.push_settings {
        bin = ("%s/%s"):format( settings.bin, id );
        data = ("%s/%s"):format( settings.bin, id );
    };
    return package;
end

function Apk.call( package, definition )
    local working_directory = working_directory();
    for _, dependency in ipairs(definition) do 
        working_directory:remove_dependency( dependency );
        package:add_dependency( dependency );
        dependency.module = package;
    end
    local android_manifest = definition.android_manifest;
    if android_manifest then 
        package:add_dependency( android_manifest );
        package.android_manifest = android_manifest;
    end
end

function Apk.build( package )
    if package:outdated() then
        local android_manifest = package.android_manifest;
        assertf( android_manifest, "Android APK '%s' does not specify 'android_manifest'", package:path() );

        local settings = package.settings;
        local aapt = ("%s/aapt"):format( settings.android.build_tools_directory );
        local resources = table.concat( settings.resources, " -S " );
        local android_jar = ("%s/platforms/%s/android.jar"):format( settings.android.sdk_directory, settings.android.sdk_platform );
        build.system( aapt, ('aapt package --auto-add-overlay -f -M "%s" -S %s -I "%s" -F "%s.unaligned"'):format(android_manifest:filename(), resources, android_jar, package:filename()) );

        pushd( ("%s/%s"):format(branch(package:filename()), basename(package:id())) );
        for file in find("") do 
            if is_file(file) then
                build.system( aapt, ('aapt add -f "%s.unaligned" "%s"'):format(relative(package:filename()), relative(file)) );
            end
        end

        local jarsigner = ("%s/bin/jarsigner"):format( settings.android.jdk_directory );
        local key = _G.key or "androiddebugkey";
        local keypass = _G.keypass or "android";
        local keystore = _G.keystore or relative( ("%s/debug.keystore"):format(package:working_directory():path()) );
        build.system( jarsigner, ("jarsigner -sigalg MD5withRSA -digestalg SHA1 -keystore %s -storepass %s %s.unaligned %s"):format(keystore, keypass, relative(package:filename()), key) );

        local zipalign = ("%s/zipalign"):format( settings.android.build_tools_directory );
        build.system( zipalign, ("zipalign -f 4 %s.unaligned %s"):format(relative(package:filename()), relative(package:filename())) );
        popd();
    end
end

function Apk.clean( package )
    rm( ("%s.unaligned"):format(package:filename()) );
    rm( package:filename() );
end

android.Apk = Apk;
