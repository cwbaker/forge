
local Aidl = build.TargetPrototype( "android.Aidl" );

function Aidl.create( settings, source )
    local working_directory = build.working_directory();
    local gen_directory = ("%s/%s"):format( settings.gen, build.relative(working_directory:path(), build.root()) );
    local aidl = build.File( ("%s/%s.java"):format(gen_directory, build.strip(source)), Aidl );
    aidl.settings = settings;
    aidl.gen_directory = gen_directory;
    aidl:add_dependency( build.SourceFile(source) );
    return aidl;
end

function Aidl.build( aidl )
    if aidl:outdated() then
        local settings = aidl.settings;
        local aidl_ = ("%s/aidl"):format( settings.android.build_tools_directory );
        local output = aidl.gen_directory;
        local platform = ("%s/platforms/%s/framework.aidl"):format( settings.android.sdk_directory, settings.android.sdk_platform );
        print( aidl:id() );
        build.system( aidl_, ([[aidl -o"%s" -p%s "%s"]]):format(output, platform, build.relative(aidl:dependency(1):filename())) );
    end
end

function Aidl.clean( aidl )
    build.rm( aidl:filename() );
end

android.Aidl = Aidl;
