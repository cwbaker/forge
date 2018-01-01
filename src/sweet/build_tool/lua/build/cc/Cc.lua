
local function depend( build, target, dependencies )
    build:merge( target, dependencies );
    local settings = target.settings;
    for _, value in ipairs(dependencies) do
        local source = build:SourceFile( value );
        source.settings = settings;

        local object = build:File( ("%s/%s/%s"):format(settings.obj_directory(target), build:relative(source:branch()), settings.obj_name(source:filename())) );
        object:add_dependency( source );
        object:add_ordering_dependency( build:Directory(object:directory()) );
        target:add_dependency( object );
    end
end

local function build_( build, target )
    local settings = target.settings;
    settings.cc( target );
end

local function create_target_prototype( id, language )
    local target_prototype = build:TargetPrototype( id );
    local function create( build, settings, architecture )
        local cc = build:Target( build:anonymous(), target_prototype );
        cc.settings = settings;
        cc.architecture = architecture or settings.default_architecture;
        cc.language = language;
        return cc;
    end
    
    target_prototype.create = create;
    target_prototype.depend = depend;
    target_prototype.build = build_;
    return target_prototype;
end

create_target_prototype( "Cc", "c" );
create_target_prototype( "Cxx", "c++" );
create_target_prototype( "ObjC", "objective-c" );
create_target_prototype( "ObjCxx", "objective-c++" );
