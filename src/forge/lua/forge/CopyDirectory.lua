
require 'forge.Copy';

local CopyDirectory = forge:TargetPrototype( "CopyDirectory" );

function CopyDirectory.create( forge, settings, identifier )
    local identifier = forge:interpolate( identifier, settings );
    local copy_directory = forge:Target( forge:anonymous(), CopyDirectory );
    copy_directory:add_ordering_dependency( forge:Directory(forge:absolute(identifier)) );
    copy_directory.settings = settings;
    return copy_directory;
end

function CopyDirectory.depend( forge, target, dependencies )
    local settings = target.settings;
    for _, value in ipairs(dependencies) do 
        local source_directory = forge:interpolate( value, settings );
        local destination_directory = target:ordering_dependency():filename();
        local cache = forge:find_target( settings.cache );
        cache:add_dependency( forge:SourceDirectory(source_directory) );

        forge:pushd( source_directory );
        for source_filename in forge:find('') do 
            if forge:is_file(source_filename) then
                local filename = forge:absolute( forge:relative(source_filename), destination_directory );
                local copy = forge:Copy (filename);
                copy:add_dependency( forge:SourceFile(source_filename) );
                target:add_dependency( copy );
            elseif forge:is_directory(source_filename) then 
                local directory = forge:SourceDirectory( source_filename );
                cache:add_dependency( directory );
            end
        end
        forge:popd();
    end
end
