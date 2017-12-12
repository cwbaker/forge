
local Plist = build.TargetPrototype( "xcode.Plist" );

function Plist.create( settings, filename, input )
    local plist = build.File( build.interpolate(filename, settings), Plist );
    plist.settings = settings;
    plist:add_dependency( build.SourceFile(input, settings) );
    plist:add_ordering_dependency( build.Directory(plist:branch()) );
    return plist;
end

function Plist.build( plist )
    if plist:outdated() then
        local command_line = {
            'plutil';
            '-convert binary1';
            ('-o "%s"'):format( plist:filename() );
            ('"%s"'):format( plist:dependency():filename() );
        };
        local plutil = plist.settings.ios.plutil;
        build.system( plutil, table.concat(command_line, " ") );
    end
end

function Plist.clean( plist )
    build.rm( plist:filename() );
end

xcode.Plist = Plist;
