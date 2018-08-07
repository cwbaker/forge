
local function depend( build, target, dependencies )
    build:merge( target, dependencies );
    local settings = target.settings;
    local replacement = settings.obj_name( target.replacement or '${obj}/%1' );
    local pattern = target.pattern or '(.-)(%.?[^%.\\/]*)$';
    for _, filename in ipairs(dependencies) do
        local source_file = build:SourceFile( filename );
        local identifier = build:root_relative( source_file ):gsub( pattern, replacement );
        local object = build:File( identifier );
        object:add_dependency( source_file );
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
    local function create( build, settings, replacement, pattern )
        local cc = build:Target( build:anonymous(), target_prototype );
        cc.settings = settings;
        cc.architecture = settings.default_architecture;
        cc.replacement = replacement;
        cc.pattern = pattern;
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
