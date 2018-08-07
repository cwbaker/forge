
require 'build.Copy';

local CopyDirectory = build:TargetPrototype( "CopyDirectory" );

function CopyDirectory.create( build, settings, identifier )
    local identifier = build:interpolate( identifier, settings );
    local copy_directory = build:Target( build:anonymous(), CopyDirectory );
    copy_directory:add_ordering_dependency( build:Directory(build:absolute(identifier)) );
    copy_directory.settings = settings;
    return copy_directory;
end

function CopyDirectory.depend( build, target, dependencies )
    local settings = target.settings;
    for _, value in ipairs(dependencies) do 
        local source_directory = build:interpolate( value, settings );
        local destination_directory = target:ordering_dependency():filename();
        local cache = build:find_target( settings.cache );
        cache:add_dependency( build:SourceDirectory(source_directory) );

        build:pushd( source_directory );
        for source_filename in build:find('') do 
            if build:is_file(source_filename) then
                local filename = build:absolute( build:relative(source_filename), destination_directory );
                local copy = build:Copy (filename) (source_filename);
                target:add_dependency( copy );
            elseif build:is_directory(source_filename) then 
                local directory = build:SourceDirectory( source_filename );
                cache:add_dependency( directory );
            end
        end
        build:popd();
    end
end
