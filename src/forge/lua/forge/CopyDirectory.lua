
local Copy = require 'forge.Copy';

local CopyDirectory = TargetPrototype( 'CopyDirectory' );

function CopyDirectory.create( toolset, identifier )
    local settings = toolset.settings;
    local identifier = toolset:interpolate( identifier, settings );
    local copy_directory = Target( toolset, anonymous(), CopyDirectory );
    copy_directory:add_ordering_dependency( toolset:Directory(absolute(identifier)) );
    copy_directory.settings = settings;
    return copy_directory;
end

function CopyDirectory.depend( toolset, target, dependencies )
    local settings = target.settings;
    for _, value in ipairs(dependencies) do 
        local source_directory = toolset:interpolate( value, settings );
        local destination_directory = target:ordering_dependency():filename();
        local cache = find_target( forge.cache );
        cache:add_dependency( toolset:SourceDirectory(source_directory) );

        pushd( source_directory );
        for source_filename in find('') do 
            if is_file(source_filename) then
                local filename = absolute( relative(source_filename), destination_directory );
                local copy = toolset:Copy (filename) {
                    source_filename;
                };
                target:add_dependency( copy[1] );
            elseif is_directory(source_filename) then 
                local directory = toolset:SourceDirectory( source_filename );
                cache:add_dependency( directory );
            end
        end
        popd();
    end
end

return CopyDirectory;
