
qt = {};

function qt.configure( settings )
    local local_settings = forge.local_settings;
    if not local_settings.qt then
        local_settings.updated = true;
        if forge:operating_system() == "windows" then
            local_settings.qt = {
                moc = "C:/Qt/5.5/bin/moc.exe";
                qt_directory = "C:/Qt/5.5";
                version = "5.5";
            };
        else
            local_settings.qt = {
                moc = forge:home( "Qt/5.5/clang_64/bin/moc" );
                qt_directory = forge:home( "Qt" );
                version = "5.5";
            };
        end
    end
end

function qt.initialize( settings )
    qt.configure( settings );
end

require "forge.qt.QtMoc";

forge:register_module( qt );
