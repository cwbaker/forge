
local Plist = build.TargetPrototype( "Plist" );

function Plist.create( settings, filename, input )
    local plist = build.File( build.interpolate(filename, settings), Plist );
    plist.settings = settings;
    plist:add_dependency( build.SourceFile(input, settings) );
    plist:add_dependency( Directory(plist:branch()) );
    return plist;
end

function Plist.build( plist )
    if plist:is_outdated() then
        local command_line = {
            ('xcrun --sdk %s plutil'):format( ios.sdkroot_by_target_and_platform(plist, platform) );
            '-convert binary1';
            ('-o "%s"'):format( plist:filename() );
            ('"%s"'):format( plist:dependency():filename() );
        };
        local xcrun = plist.settings.ios.xcrun;
        build.system( xcrun, table.concat(command_line, " ") );
    end
end

function Plist.clean( plist )
    rm( plist:filename() );
end

xcode.Plist = Plist;
