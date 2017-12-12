
PlistPrototype = TargetPrototype { "Plist" };

function PlistPrototype.build( plist )
    if plist:is_outdated() then
        local command_line = {
            'xcrun --sdk %s plutil' % ios.SDK_BY_PLATFORM[platform];
            '-convert binary1';
            '-o "%s"' % plist:get_filename();
            '"%s"' % plist.source:get_filename();
        };
        local xcrun = plist.settings.ios.xcrun;
        build.system( xcrun, table.concat(command_line, " ") );
    end
end

function Plist( destination )
    build.begin_target();
    return function( source )
        return build.end_target( function()
            local settings = build.current_settings();
            destination = build.interpolate( destination, settings );
            if type(source) == "function" then
                source = build.expand_target( source )[1];
            end
            if type(source) == "string" then
                source = file( build.interpolate(source, settings) );
            end
            local plist = target( destination, PlistPrototype );
            plist:set_filename( destination );
            plist:add_dependency( source );
            plist:add_dependency( Directory(branch(destination)) );
            plist.source = source;
            plist.settings = settings;
            return { plist };
        end );
    end
end
