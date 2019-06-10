
local Jar = forge:FilePrototype( 'Jar' );

function Jar.build( forge, target )
    local arguments = 'cvf';
    local jar = native( ('%s/bin/jar'):format(forge.settings.android.jdk_directory) );
    for _, dependency in target:dependencies() do 
        local classes = {};
        pushd( dependency:ordering_dependency() );
        for _, filename in dependency:filenames() do 
            table.insert( classes, relative(filename, directory) );
        end
        if #classes > 0 then
            forge:system( jar, ('jar %s "%s" "%s"'):format(arguments, target, table.concat(classes, '" "')) );
            arguments = 'uvf';
        end
        popd();
    end
end

return Jar;
