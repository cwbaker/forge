
local QtMoc = forge:TargetPrototype( 'QtMoc' );

function QtMoc.build( forge, target )
    local moc = target.settings.qt.moc;
    local command_line = {
        'moc',
        ('-o "%s"'):format( forge:relative(target) ),
        ('"%s"'):format( target:dependency() )
    };
    forge:system( moc, command_line );
end
