
Directory = build.TargetPrototype( "Directory" );

function Directory.create( settings, id )
	return build.File( id, Directory );
end

function Directory.build( directory )
    if directory:outdated() then
        build.mkdir( directory:filename() );
    end
end

function Directory.clean( directory )
end
