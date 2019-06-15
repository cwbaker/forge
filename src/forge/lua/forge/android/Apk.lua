
local Apk = forge:FilePrototype( 'Apk' );

function Apk.build( forge, target )
    local files = {};
    local resources = {};
    local android_manifest;
    for _, dependency in target:dependencies() do 
        if dependency:prototype() == forge.Ivy then 
            for _, archive in dependency:implicit_dependencies() do 
                if extension(archive) ~= '.jar' and exists(('%s/res'):format(archive)) then
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
    assertf( android_manifest and leaf(android_manifest:filename()) == "AndroidManifest.xml", "Android APK '%s' does not specify a manifest named 'AndroidManifest.xml'", target:path() );

    local settings = forge.settings;
    local aapt = ("%s/aapt"):format( settings.android.build_tools_directory );
    local android_jar = ("%s/platforms/%s/android.jar"):format( settings.android.sdk_directory, settings.android.sdk_platform );
    system( aapt, {
        'aapt',
        'package',
        '--auto-add-overlay',
        '-f',
        ('-M "%s"'):format( android_manifest ),
        table.concat( resources, ' ' );
        ('-I "%s"'):format( android_jar ),
        ('-F "%s.unaligned"'):format( target )
    } );

    pushd( ("%s/%s"):format(branch(target), basename(target)) );
    for _, dependency in ipairs(files) do 
        system( aapt, {
            "aapt",
            'add',
            ('-f "%s.unaligned"'):format( relative(target) ),
            ('"%s"'):format( relative(dependency) )
        } );
    end

    local jarsigner = ("%s/bin/jarsigner"):format( settings.android.jdk_directory );
    local key = _G.key or "androiddebugkey";
    local keypass = _G.keypass or "android";
    local keystore = _G.keystore or relative( ("%s/debug.keystore"):format(target:working_directory():path()) );
    system( jarsigner, {
        'jarsigner',
        '-sigalg MD5withRSA',
        '-digestalg SHA1',
        ('-keystore %s'):format( keystore ),
        ('-storepass %s'):format( keypass ),
        ('%s.unaligned'):format( relative(target) ),
        ('%s'):format( key )
    } );

    local zipalign = ("%s/zipalign"):format( settings.android.build_tools_directory );
    system( zipalign, {
        'zipalign',
        '-f 4',
        ('%s.unaligned'):format( relative(target) ),
        ('%s'):format( relative(target) )
    } );
    forge:popd();
end

function Apk.clean( forge, target )
    rm( ("%s.unaligned"):format(target:filename()) );
    rm( target );
end

return Apk;
