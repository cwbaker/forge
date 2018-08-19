
local Apk = forge:TargetPrototype( 'Apk' );

function Apk.build( forge, target )
    local files = {};
    local resources = {};
    local android_manifest;
    for _, dependency in target:dependencies() do 
        if dependency:prototype() == forge.Ivy then 
            for _, archive in dependency:implicit_dependencies() do 
                if forge:extension(archive) ~= '.jar' and forge:exists(('%s/res'):format(archive)) then
                    table.insert( resources, ('-S "%s/res"'):format(archive) );
                end
            end
        elseif dependency:id() == 'res' then
            table.insert( resources, ('-S "%s"'):format(dependency) );
        elseif dependency:id() == 'AndroidManifest.xml' then
            android_manifest = dependency;
        elseif dependency:filename() ~= '' then
            table.insert( files, dependency );
        else
            for _, shared_library in dependency:dependencies() do 
                table.insert( files, shared_library );
            end
        end
    end
    assertf( android_manifest and forge:leaf(android_manifest:filename()) == "AndroidManifest.xml", "Android APK '%s' does not specify a manifest named 'AndroidManifest.xml'", target:path() );

    local settings = target.settings;
    local aapt = ("%s/aapt"):format( settings.android.build_tools_directory );
    local android_jar = ("%s/platforms/%s/android.jar"):format( settings.android.sdk_directory, settings.android.sdk_platform );
    forge:system( aapt, {
        'aapt',
        'package',
        '--auto-add-overlay',
        '-f',
        ('-M "%s"'):format( android_manifest ),
        table.concat( resources, ' ' );
        ('-I "%s"'):format( android_jar ),
        ('-F "%s.unaligned"'):format( target )
    } );

    forge:pushd( ("%s/%s"):format(forge:branch(target), forge:basename(target)) );
    for _, dependency in ipairs(files) do 
        forge:system( aapt, {
            "aapt",
            'add',
            ('-f "%s.unaligned"'):format( forge:relative(target) ),
            ('"%s"'):format( forge:relative(dependency) )
        } );
    end

    local jarsigner = ("%s/bin/jarsigner"):format( settings.java.jdk_directory );
    local key = _G.key or "androiddebugkey";
    local keypass = _G.keypass or "android";
    local keystore = _G.keystore or forge:relative( ("%s/debug.keystore"):format(target:working_directory():path()) );
    forge:system( jarsigner, {
        'jarsigner',
        '-sigalg MD5withRSA',
        '-digestalg SHA1',
        ('-keystore %s'):format( keystore ),
        ('-storepass %s'):format( keypass ),
        ('%s.unaligned'):format( forge:relative(target) ),
        ('%s'):format( key )
    } );

    local zipalign = ("%s/zipalign"):format( settings.android.build_tools_directory );
    forge:system( zipalign, {
        'zipalign',
        '-f 4',
        ('%s.unaligned'):format( forge:relative(target) ),
        ('%s'):format( forge:relative(target) )
    } );
    forge:popd();
end

function Apk.clean( forge, target )
    forge:rm( ("%s.unaligned"):format(target:filename()) );
    forge:rm( target );
end

android.Apk = Apk;