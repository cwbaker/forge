
local Dex = build.TargetPrototype( "android.Dex" );

function Dex:depend( dependencies )
    local proguard = dependencies.proguard;
    if proguard then 
        self:add_dependency( proguard );
        self.proguard = proguard;
        dependencies.proguard = nil;
    end

    local jars = dependencies.jars;
    if jars then 
        java.add_jar_dependencies( self, dependencies.jars );
        dependencies.jars = nil;
    end

    return build.default_depend( self, dependencies );
end

function Dex:build()
    local jars = {};

    if self.proguard and self.settings.android.proguard_enabled then 
        local proguard = self.proguard; 
        local proguard_sh = ("%s/tools/proguard/bin/proguard.sh"):format( self.settings.android.sdk_directory );
        build.system( proguard_sh, ('proguard.sh -printmapping \"%s/%s.map\" "@%s"'):format(build.classes_directory(self), build.leaf(self), proguard:filename()) );
        table.insert( jars, ('\"%s/classes.jar\"'):format(build.classes_directory(self)) );
    else
        table.insert( jars, build.classes_directory(self) );
    end

    local third_party_jars = self.third_party_jars;
    if third_party_jars then 
        for _, jar in ipairs(third_party_jars) do 
            table.insert( jars, jar );
        end
    end
    
    local third_party_jars = self.settings.third_party_jars 
    if third_party_jars then
        for _, jar in ipairs(third_party_jars) do 
            table.insert( jars, jar );
        end
    end

    for _, dependency in self:dependencies() do 
        if dependency:prototype() == build.Jar then 
            table.insert( jars, build.relative(dependency:filename()) );
        end
    end

    local dx = build.native( ("%s/dx"):format(self.settings.android.build_tools_directory) );
    if build.operating_system() == "windows" then
        dx = ("%s.bat"):format( dx );
    end
    build.shell( {
        ('\"%s\"'):format( dx ),
        '--dex',
        '--verbose',
        ('--output=\"%s\"'):format( self ), 
        ('%s'):format( table.concat(jars, " ") )
    } );
end

android.Dex = Dex;
