
local Lipo = build.TargetPrototype( "Lipo" );

function Lipo.create( settings, id )
    local lipo = build.Target( id, Lipo );
    lipo.settings = settings;
    lipo:set_filename( ("%s/%s"):format(settings.bin, id) );
    working_directory():add_dependency( lipo );
    return lipo;
end

function Lipo.call( lipo, definition )
    local working_directory = working_directory();
    for _, dependency in ipairs(definition) do 
        working_directory:remove_dependency( dependency );
        lipo:add_dependency( dependency );
        dependency.module = lipo;
    end
end

function Lipo.build( self )
    if self:outdated() then
        lipo_executable( self );
    end
end

function Lipo.clean( self )
    clean_executable( self );
end

xcode.Lipo = Lipo;
