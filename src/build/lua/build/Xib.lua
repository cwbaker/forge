
XibPrototype = TargetPrototype { "Xib" };

function XibPrototype.build( xib )
    if xib:is_outdated() then
        local sdk = ios.SDK_BY_PLATFORM[platform];
        local xcrun = xib.settings.ios.xcrun;
        for dependency in xib:get_dependencies() do 
            if dependency:is_outdated() and dependency:prototype() == nil then                
                build.system( xcrun, [[xcrun --sdk %s ibtool --output-format binary1 --compile "%s" "%s"]] % {
                    sdk, 
                    dependency:get_filename(),
                    dependency.source
                } );
            end
        end
    end
end

function XibPrototype.clean( xib )
    for dependency in xib:get_dependencies() do
        if dependency:prototype() == nil then
            rm( dependency:path() );
        end
    end
end

function Xib( definition )
    build.begin_target();
    return build.end_target( function()
        local xibs = {};
        local settings = build.push_settings( definition.settings );
        if build.built_for_platform_and_variant(settings) then
            xib = target( "", XibPrototype, definition );
            xib.settings = settings;

            local directory = Directory( settings.data );

            for _, value in ipairs(xib) do
                if type(value) == "string" then
                    local xib_file = file( value );
                    xib_file:set_required_to_exist( true );
                    xib_file.unit = xib;
                    xib_file.settings = settings;

                    local nib_file = file( "%s/%s.nib" % {settings.data, basename(value)} );
                    nib_file.source = value;
                    xib.nib_file = nib_file;
                    nib_file:add_dependency( xib_file );
                    nib_file:add_dependency( directory );
                    xib:add_dependency( nib_file );
                elseif type(value) == "function" then
                    local dependency = build.expand_target( value );
                    xib:add_dependency( dependency );
                end
            end
            table.insert( xibs, xib );
        end        
        build.pop_settings();
        return xibs;
    end);
end
