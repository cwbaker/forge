
local function depend( build, target, dependencies )
    build:merge( target, dependencies );
    local replacement = target.replacement or '${object %1}';
    local pattern = target.pattern or '(.*)(%..*)$';
    local settings = target.settings;
    for _, source_filename in ipairs(dependencies) do
        local source = build:SourceFile( source_filename );
        local filename = source_filename:gsub( pattern, replacement );
        local object = build:File( filename );
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
