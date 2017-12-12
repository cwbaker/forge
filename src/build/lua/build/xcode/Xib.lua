
local Xib = build.TargetPrototype( "Xib" );

function Xib.create( settings, definition )
    local xib = build.Target( "", Xib, definition );
    xib.settings = settings;
    local directory = Directory( settings.data );
    for _, value in ipairs(xib) do
        local xib_file = build.SourceFile( value, settings );
        xib_file:set_required_to_exist( true );
        xib_file.unit = xib;
        xib_file.settings = settings;

        local nib_file = file( ("%s/%s.nib"):format(settings.data, basename(value)) );
        nib_file.source = value;
        xib.nib_file = nib_file;
        nib_file:add_dependency( xib_file );
        nib_file:add_dependency( directory );
        xib:add_dependency( nib_file );
    end
    return xib;
end

function Xib.build( xib )
    if xib:outdated() then
        local sdk = ios.sdkroot_by_target_and_platform( xib, platform );
        local xcrun = xib.settings.ios.xcrun;
        for dependency in xib:dependencies() do 
            if dependency:is_outdated() and dependency:prototype() == nil then                
                build.system( xcrun, ([[xcrun --sdk %s ibtool --output-format binary1 --compile "%s" "%s"]]):format(
                    sdk, 
                    dependency:filename(),
                    dependency:dependency():filename()
                ) );
            end
        end
    end
end

function Xib.clean( xib )
    for dependency in xib:dependencies() do
        if dependency:prototype() == nil then
            rm( dependency:path() );
        end
    end
end

xcode.Xib = Xib;
