
-- Visit a target by calling a member function "clean" if it exists or if
-- there is no "clean" function and the target is not marked as a source file
-- that must exist then its associated file is deleted.
local function clean_visit( target )
    local clean_function = target.clean;
    if clean_function then 
        clean_function( target.forge_, target );
    elseif target:cleanable() and target:filename() ~= "" then 
        forge:rm( target:filename() );
    end
end

-- Visit a target by calling a member function "build" if it exists and 
-- setting that Target's built flag to true if the function returns with
-- no errors.
local function build_visit( target )
    local build_function = target.build;
    if build_function and target:outdated() then 
        local filename = target:filename();
        if filename ~= "" then
            printf( forge:leaf(filename) );
        end
        target:clear_implicit_dependencies();
        local success, error_message = pcall( build_function, target.forge_, target );
        target:set_built( success );
        if not success then 
            clean_visit( target );
            assert( success, error_message );
        end
    end
end

function default()
    local failures = forge:postorder( forge:find_initial_target(goal), build_visit );
    forge:save();
    printf( "forge: default (build)=%dms", math.ceil(forge:ticks()) );
    return failures;
end

function clean()
    local failures = forge:postorder( forge:find_initial_target(goal), clean_visit );
    printf( "forge: clean=%sms", tostring(math.ceil(forge:ticks())) );
    return failures;
end

function reconfigure()
    rm( forge.settings.local_settings_filename );
    forge:find_initial_target();
    return 0;
end

function dependencies()
    forge:print_dependencies( forge:find_initial_target(goal) );
    return 0;
end

function namespace()
    forge:print_namespace( forge:find_initial_target(goal) );
    return 0;
end
