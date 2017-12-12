
Directory = build.TargetPrototype( "Directory" );

function Directory.create( _, id )
	return build.File( id, Directory );
end

function Directory.build( directory )
    if directory:is_outdated() then
        mkdir( directory:get_filename() );
    end
end
