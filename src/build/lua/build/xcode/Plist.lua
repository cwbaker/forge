
local Plist = build.TargetPrototype( "Plist" );

function Plist.create( _, filename, input, settings )
    local settings = settings or build.current_settings();
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
            ('-o "%s"'):format( plist:get_filename() );
            ('"%s"'):format( plist:dependency():get_filename() );
        };
        local xcrun = plist.settings.ios.xcrun;
        build.system( xcrun, table.concat(command_line, " ") );
    end
end

function Plist.clean( plist )
    rm( plist:get_filename() );
end

xcode.Plist = Plist;
