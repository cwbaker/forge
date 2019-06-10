
local Copy = require 'forge.Copy';

local CopyDirectory = forge:TargetPrototype( 'CopyDirectory' );

function CopyDirectory.create( forge, identifier )
    local settings = forge.settings;
    local identifier = forge:interpolate( identifier, settings );
    local copy_directory = forge:Target( anonymous(), CopyDirectory );
    copy_directory:add_ordering_dependency( forge:Directory(absolute(identifier)) );
    copy_directory.settings = settings;
    return copy_directory;
end

function CopyDirectory.depend( forge, target, dependencies )
    local settings = target.settings;
    for _, value in ipairs(dependencies) do 
        local source_directory = forge:interpolate( value, settings );
        local destination_directory = target:ordering_dependency():filename();
        local cache = find_target( forge.cache );
        cache:add_dependency( forge:SourceDirectory(source_directory) );

        pushd( source_directory );
        for source_filename in find('') do 
            if is_file(source_filename) then
                local filename = absolute( relative(source_filename), destination_directory );
                local copy = forge:Copy (filename) {
                    source_filename;
                };
                target:add_dependency( copy );
            elseif is_directory(source_filename) then 
                local directory = forge:SourceDirectory( source_filename );
                cache:add_dependency( directory );
            end
        end
        popd();
    end
end

return CopyDirectory;
