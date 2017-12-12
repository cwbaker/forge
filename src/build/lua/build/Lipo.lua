
LipoPrototype = TargetPrototype { "Lipo" };

function LipoPrototype.static_depend( self )
    build.add_library_dependencies( self );
end

function LipoPrototype.build( self )
    if self:is_outdated() then
        lipo_executable( self );
    end
end

function LipoPrototype.clean( self )
    clean_executable( self );
end

function Lipo( id )
    build.begin_target();
    return function( dependencies )
        return build.end_target( function()
            local lipos = {};
            local settings = build.push_settings( dependencies.settings or {} );
            if build.built_for_platform_and_variant(settings) then
                local lipo = target( id, LipoPrototype, {settings = settings} );
                local filename = "%s/%s" % { settings.bin, id };
                settings = build.push_settings {
                    bin = "%s" % obj_directory( lipo );
                };
                build.add_package_dependencies( lipo, filename, dependencies );
                build.pop_settings();
                table.insert( lipos, lipo );
            end
            build.pop_settings();
            return lipos;
        end);
    end
end
