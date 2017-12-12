
local Apk = build:TargetPrototype( "android.Apk" );

function Apk.build( build, target )
    local android_manifest = target:dependency( 1 );
    assertf( android_manifest and build:leaf(android_manifest:filename()) == "AndroidManifest.xml", "Android APK '%s' does not specify 'AndroidManifest.xml' as its first dependency'", target:path() );

    local settings = target.settings;
    local aapt = ("%s/aapt"):format( settings.android.build_tools_directory );
    local resources = table.concat( settings.resources, " -S " );
    local android_jar = ("%s/platforms/%s/android.jar"):format( settings.android.sdk_directory, settings.android.sdk_platform );
    build:system( aapt, {
        'aapt',
        'package',
        '--auto-add-overlay',
        '-f',
        ('-M "%s"'):format( android_manifest ),
        ('-S %s'):format( resources ),
        ('-I "%s"'):format( android_jar ),
        ('-F "%s.unaligned"'):format( target )
    } );

    build:pushd( ("%s/%s"):format(build:branch(target), build:basename(target)) );
    for dependency in build:walk_dependencies(target, 2) do 
        build:system( aapt, {
            "aapt",
            'add',
            ('-f "%s.unaligned"'):format( build:relative(target) ),
            ('"%s"'):format( build:relative(dependency) )
        } );
    end

    local jarsigner = ("%s/bin/jarsigner"):format( settings.java.jdk_directory );
    local key = _G.key or "androiddebugkey";
    local keypass = _G.keypass or "android";
    local keystore = _G.keystore or build:relative( ("%s/debug.keystore"):format(target:working_directory():path()) );
    build:system( jarsigner, {
        'jarsigner',
        '-sigalg MD5withRSA',
        '-digestalg SHA1',
        ('-keystore %s'):format( keystore ),
        ('-storepass %s'):format( keypass ),
        ('%s.unaligned'):format( build:relative(target) ),
        ('%s'):format( key )
    } );

    local zipalign = ("%s/zipalign"):format( settings.android.build_tools_directory );
    build:system( zipalign, {
        'zipalign',
        '-f 4',
        ('%s.unaligned'):format( build:relative(target) ),
        ('%s'):format( build:relative(target) )
    } );
    build:popd();
end

function Apk.clean( build, target )
    build:rm( ("%s.unaligned"):format(target:filename()) );
    build:rm( target );
end

android.Apk = Apk;
