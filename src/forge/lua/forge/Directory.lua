
local Directory = forge:TargetPrototype( "Directory" );

function Directory.create( forge, settings, identifier )
	return forge:File( identifier, Directory );
end

function Directory.build( forge, target )
    forge:mkdir( target );
end

function Directory.clean( forge, target )
end
