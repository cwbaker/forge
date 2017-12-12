
local Directory = build:TargetPrototype( "Directory" );

function Directory.create( build, settings, identifier )
	return build:File( identifier, Directory );
end

function Directory.build( build, target )
    build:mkdir( target );
end

function Directory.clean( build, target )
end
