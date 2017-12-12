
local QtMoc = build:TargetPrototype( "qt.QtMoc" );

function QtMoc.build( build, target )
    local moc = target.settings.qt.moc;
    local command_line = {
        'moc',
        ('-o "%s"'):format( build:relative(target) ),
        ('"%s"'):format( target:dependency() )
    };
    build:system( moc, command_line );
end
