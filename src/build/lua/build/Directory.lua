
Directory = build.TargetPrototype( "Directory" );

function Directory.create( settings, id )
	return build.File( id, Directory );
end

function Directory.build( directory )
    if directory:is_outdated() then
        mkdir( directory:filename() );
    end
end

function Directory.clean( directory )
end
