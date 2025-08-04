forge = _G.forge or {};

-- Provide printf().
function printf(format, ...)
    print(string.format(format, ...));
end

-- Provide formatted assert().
function assertf(condition, format, ...)
    if not condition then
        assert(condition, string.format(format, ...));
    end
end

-- Default action calling through to `build()`.
function default()
    return build();
end

-- Call prepare on targets that provide it.
function prepare_visit(target)
    local prepare_function = target.prepare;
    if prepare_function then
        return prepare_function(target.toolset, target);
    end
end

-- Visit a target by calling a member function "clean" if it exists or if
-- there is no "clean" function and the target is not marked as a source file
-- that must exist then its associated file is deleted.
function clean_visit(target)
    local clean_function = target.clean;
    if clean_function then
        clean_function(target.toolset, target);
    elseif target:cleanable() then
        for _, filename in target:filenames() do
            if filename ~= '' then
                rm(filename);
            end
        end
        target:clear_filenames();
        target:set_built(false);
    end
end

-- Visit a target by calling a member function "build" if it exists and
-- setting that Target's built flag to true if the function returns with
-- no errors.
function build_visit(target)
    if target:outdated() then
        local build_function = target.build;
        if build_function then
            local success, error_message = pcall(build_function, target.toolset, target);
            target:set_built(success);
            if not success then
                clean_visit(target);
                assert(success, error_message);
            end
        else
            target:set_built(true);
        end
    end
end

-- Prepare dependency graph by propagating transitive dependencies.
function prepare(target)
    local failures = preorder(target, prepare_visit);
    return failures;
end

-- Build action.
function build()
    local target = find_initial_target(goal);
    local failures = prepare(target) + postorder(target, build_visit);
    forge:save();
    printf("forge: default (build)=%dms", math.ceil(ticks()));
    return failures;
end

-- Clean action.
function clean()
    local failures = postorder(find_initial_target(goal), clean_visit);
    forge:save();
    printf("forge: clean=%sms", tostring(math.ceil(ticks())));
    return failures;
end

-- Provide global reconfigure command.
function reconfigure()
    local cache_directory = forge.cache_directory;
    local filename = cache_directory and root(('%s/local_settings.lua'):format(cache_directory)) or root('local_settings.lua');
    rm(filename);
    return 0;
end

-- Dependencies action.
function dependencies()
    local target = find_initial_target(goal);
    local failures = prepare(target);
    print_dependencies(target);
    return failures;
end

-- Namespace action.
function namespace()
    local target = find_initial_target(goal);
    local failures = prepare(target);
    print_namespace(target);
    return failures;
end

-- Help action.
function help()
    printf [[
Variables:
  goal={goal}        Target to build, default is all.
  variant={variant}  Variant to build, default is debug.
Commands:
  build              Build outdated targets.
  clean              Clean all targets.
  reconfigure        Re-run auto-detected configuration.
  dependencies       Print dependency hierarchy.
  namespace          Print namespace hierarchy.
]];
end

-- Iterate over toolsets that match patterns.
function toolsets(...)
    local toolsets_iterator = coroutine.wrap(function (...)
        coroutine.yield();
        for i = 1, select('#', ...) do
            local pattern = select(i, ...);
            if pattern and pattern ~= '' then
                for _, toolset, identifier in all_toolsets() do
                    if identifier:find(pattern) then
                        coroutine.yield(i, toolset);
                    end
                end
            else
                for _, toolset, identifier in all_toolsets() do
                    coroutine.yield(i, toolset);
                end
            end
        end

    end);

    -- Resume the coroutine with the variable length arguments passed to this
    -- function.  It yields immediately, the coroutine is returned, and
    -- then subsequent yields return toolsets to the caller in a loop.
    toolsets_iterator(...);
    return toolsets_iterator;
end

-- Execute command raising an error if it doesn't return 0.
function run(command, arguments, environment, dependencies_filter, stdout_filter, stderr_filter, ...)
    if type(arguments) == 'table' then
        arguments = table.concat(arguments, ' ');
    end
    if execute(command, arguments, environment, dependencies_filter, stdout_filter, stderr_filter, ...) ~= 0 then
        error(('%s failed'):format(arguments), 0);
    end
end

-- Execute a command through the host system's native shell - either
-- "C:/windows/system32/cmd.exe" on Windows system or "/bin/sh" anywhere else.
function shell(arguments, dependencies_filter, stdout_filter, stderr_filter, ...)
    if type(arguments) == 'table' then
        arguments = table.concat(arguments, ' ');
    end
    if operating_system() == 'windows' then
        local cmd = 'C:/windows/system32/cmd.exe';
        local result = execute(cmd, ('cmd /c "%s"'):format(arguments), dependencies_filter, stdout_filter, stderr_filter, ...);
        assertf(result == 0, '[[%s]] failed (result=%d)', arguments, result);
    else
        local sh = '/bin/sh';
        local result = execute(sh, ('sh -c "%s"'):format(arguments), dependencies_filter, stdout_filter, stderr_filter, ...);
        assertf(result == 0, '[[%s]] failed (result=%d)', arguments, tonumber(result));
    end
end

-- Return a value from a table using the first key as a lookup.
function switch(values)
    assert(values[1] ~= nil, "No value passed to `switch()`");
    return values[values[1]];
end

-- Express *path* relative to the root directory.
function root_relative(path)
    return relative(absolute(path), root());
end

-- Make *path* absolute relative to the calling Lua script.
function script(path)
    local script_directory = branch(debug.getinfo(2, 'S').source:sub(2));
    return absolute(path, script_directory);
end

-- Find first existing file named *filename* in *paths*.
--
-- Searching is not performed when *filename* is an absolute path.  In this
-- case *filename* is returned immediately if it names an existing file.
--
-- The *paths* variable can be a string containing a `:` or `;` delimited list
-- of paths or a table containing those paths.  If *paths* is nil then its
-- default value is set to that returned by `os.getenv('PATH')`.
--
-- Returns the first file named *filename* that exists at a directory listed
-- in *paths* or nothing if no existing file is found.
function which(filename, paths)
    local paths = paths or os.getenv('PATH');
    local separator_pattern = '[^:]+';
    if operating_system() == 'windows' then
        separator_pattern = '[^;]+';
        if extension(filename) == '' then
            filename = ('%s.exe'):format(filename);
        end
    end
    if type(paths) == 'string' then
        if is_absolute(filename) then
            if exists(filename) then
                return filename;
            end
        else
            for directory in paths:gmatch(separator_pattern) do
                local path = ('%s/%s'):format(directory, filename);
                if exists(path) then
                    return path;
                end
            end
        end
    elseif type(paths) == 'table' then
        for _, directory in ipairs(paths) do
            local path = ('%s/%s'):format(directory, filename);
            if exists(path) then
                return path;
            end
        end
    end
end

-- Find and return the initial target to forge.
--
-- If *goal* is nil or empty then the initial target is the first all target
-- that is found in a search up from the current working directory to the
-- root directory.
--
-- Otherwise if *goal* is specified then the target that matches *goal*
-- exactly and has at least one dependency or the target that matches
-- `${*goal*}/all` is returned.  If neither of those targets exists then nil
-- is returned.
function find_initial_target(goal)
    if not goal or goal == '' then
        local goal = initial();
        local all = find_target(('%s/all'):format(goal));
        while not all and goal ~= '' do
            goal = branch(goal);
            all = find_target(('%s/all'):format(goal));
        end
        return all;
    end

    local goal = initial(goal);
    local all = find_target(goal);
    if all and all:dependency() then
        return all;
    end

    local all = find_target(('%s/all'):format(goal));
    if all and all:dependency() then
        return all;
    end
    return nil;
end


-- Decode file access output from the hooks library.
--
-- Matches lines starting with "^==", an access mode, and a full path
-- that are returned from the hooks library that intercepts file access
-- from spawned processes.  Access mode is "read" or "write" and the
-- full path is the full path to the file accessed.  Files opened for
-- read can be assumed to be dependencies.  Files opened for writing can be
-- assumed to be outputs.
--
-- Returns access and path to file or nil for lines that don't match the
-- pattern.
function decode_access(line)
    local ACCESS_PATTERN = "^== (%a+) '([^']*)'";
    local access, path = line:match(ACCESS_PATTERN);
    return access, path;
end

function FileRule(identifier, identify)
    local identify = identify or Toolset.interpolate;
    local file_rule = Rule(identifier);
    file_rule.create = function (toolset, identifier, rule)
        local identifier, filename = identify(toolset, identifier);
        local target = Target(toolset, identifier, rule);
        target:set_filename(filename or target:path());
        target:set_cleanable(true);
        target:add_ordering_dependency(toolset:Directory(branch(target)));
        local created = target.created;
        if created then
            created(toolset, target);
        end
        return target;
    end
    return file_rule;
end

function JavaStyleRule(identifier, pattern)
    local output_directory_modifier = Toolset.interpolate;
    local pattern = pattern or '(.-([^\\/]-))%.?([^%.\\/]*)$';
    local java_style_rule = Rule(identifier);
    function java_style_rule.create(toolset, output_directory, rule)
        local output_directory = root_relative():gsub(pattern, output_directory_modifier(toolset, output_directory));
        local target = Target(toolset, anonymous(), rule);
        target:set_cleanable(true);
        target:add_ordering_dependency(toolset:Directory(output_directory));
        local created = target.created;
        if created then
            created(toolset, target);
        end
        return target;
    end
    return java_style_rule;
end

function PatternRule(identifier, identify, pattern)
    local identify = identify or Toolset.interpolate;
    local pattern = pattern or '(.-([^\\/]-))%.?([^%.\\/]*)$';
    local pattern_rule = Rule(identifier);
    pattern_rule.create = function (toolset, replacement)
        local targets = {};
        local replacement = toolset:interpolate(replacement);
        local targets_metatable = {
            __call = function (targets, dependencies)
                local attributes = merge({}, dependencies);
                for _, filename in walk_tables(dependencies) do
                    local source_file = toolset:SourceFile(filename);
                    local identifier, filename = identify(toolset, root_relative(source_file):gsub(pattern, replacement));
                    local target = Target(toolset, identifier, pattern_rule);
                    target:set_filename(filename or target:path());
                    target:set_cleanable(true);
                    target:add_ordering_dependency(toolset:Directory(branch(target)));
                    merge(target, attributes);
                    local created = target.created;
                    if created then
                        created(toolset, target);
                    end
                    target:add_dependency(source_file);
                    table.insert(targets, target);
                end
                return targets;
            end
        };
        setmetatable(targets, targets_metatable);
        return targets;
    end
    return pattern_rule;
end

function GroupRule(identifier, identify, pattern)
    local identify = identify or Toolset.interpolate;
    local pattern = pattern or '(.-([^\\/]-))%.?([^%.\\/]*)$';
    local group_rule = Rule(identifier);
    group_rule.create = function (toolset, replacement)
        local targets = {};
        local replacement = toolset:interpolate(replacement);
        local targets_metatable = {
            __call = function (targets, dependencies)
                local target = targets[1];
                merge(target, dependencies);
                for _, filename in walk_tables(dependencies) do
                    local source_file = toolset:SourceFile(filename);
                    local identifier, filename = identify(toolset, root_relative(source_file):gsub(pattern, replacement));
                    local file = Target(toolset, identifier);
                    file:set_filename(filename or file:path());
                    file:set_cleanable(true);
                    file:add_ordering_dependency(toolset:Directory(branch(file)));
                    file:add_dependency(source_file);
                    local created = target.created;
                    if created then
                        created(toolset, target);
                    end
                    target:add_dependency(file);
                end
                return targets;
            end
        };
        local target = Target(toolset, anonymous(), group_rule);
        table.insert(targets, target);
        setmetatable(targets, targets_metatable);
        return targets;
    end
    return group_rule;
end

-- Recursively walk the tables passed in *dependencies* until reaching targets
-- or non-table values (e.g. strings) that are yielded back to the caller.
function walk_tables(dependencies)
    local function typename(value)
        if type(value) == 'table' then
            local metatable = getmetatable(value);
            return metatable and metatable.__name;
        end
    end

    local index = 1;
    local function walk(values)
        for _, value in ipairs(values) do
            if type(value) ~= 'table' or typename(value) == 'forge.Target' then
                coroutine.yield(index, value);
                index = index + 1;
            else
                walk(value);
            end
        end
    end

    return coroutine.wrap(function ()
        walk(dependencies);
    end);
end

-- Recursively walk the dependencies of *target* until a target with a
-- filename is reached.
function walk_dependencies(target, yield_recurse, dependencies)
    local yield_recurse = yield_recurse or function (dependency)
        local yield = dependency:filename() ~= '';
        local recurse = not yield;
        return yield, recurse;
    end;
    local dependencies = dependencies or Target.dependencies;
    local index = 1;
    local function walk(target, depth)
        for _, dependency in dependencies(target) do
            local yield, recurse = yield_recurse(dependency)
            if yield then
                coroutine.yield(index, dependency, depth);
                index = index + 1;
            end
            if recurse then
                walk(dependency, depth + 1);
            end
        end
    end
    return coroutine.wrap(function ()
        walk(target, 0);
    end);
end

function walk_all_dependencies(target, yield_recurse)
    return walk_dependencies(target, yield_recurse, Target.all_dependencies);
end

-- Merge fields with string keys from source to destination.
--
-- Returns destination.
function merge(destination, source)
    local destination = destination or {};
    for key, value in pairs(source) do
        if type(key) == 'string' then
            if type(value) == 'table' then
                local values = destination[key] or {};
                for _, other_value in ipairs(value) do
                    table.insert(values, other_value);
                end
                destination[key] = values;
            else
                destination[key] = value;
            end
        end
    end
    return destination;
end

-- Recursively copy a tree of Lua tables.
--
-- Destination tables are created as necessary.  When source or destination
-- types aren't both tables the the source value is copied to the
-- destination.
--
-- Returns destination, newly allocated if destination was nil on input.
function apply(destination, source)
    if source then
        local destination = destination or {};
        for key, value in pairs(source) do
            local destination_value = destination[key];
            if type(value) ~= 'table' or type(destination_value) ~= 'table' then
                destination[key] = value;
            else
                destination[key] = apply(destination_value, value);
            end
        end
        return destination;
    end
    return destination;
end

-- Load cached dependencies and local settings.
--
-- Cached dependencies are loaded from the file named *.forge* in the root
-- directory of the project or the file named *${variant}/.forge* if the
-- variables `variant` or `forge.variant` are set.
--
-- Local settings are loaded from the file *local_settings.lua* in the root
-- directory of the project if it exists or set to an empty table otherwise.
--
-- Returns self
function forge:load(cache_directory)
    if not self.loaded then
        if cache_directory then
            local local_settings = root(('%s/local_settings.lua'):format(cache_directory));
            self.cache_directory = cache_directory;
            self.local_settings = exists(local_settings) and dofile(local_settings) or {};
            self.cache = root(('%s/.forge'):format(cache_directory));
        else
            local local_settings = root('local_settings.lua');
            self.local_settings = exists(local_settings) and dofile(local_settings) or {};
            self.cache = root('.forge');
        end
        self.loaded = true;
        load_binary(self.cache);
    end
    return self;
end

-- Save the dependency graph and local settings.
function forge:save()
    -- Serialize values to to a Lua file (typically the local settings table).
    local function serialize(file, value, level)
        local function indent(level)
            for i = 1, level do
                file:write('  ');
            end
        end

        if level == 0 then
            file:write('\nreturn ');
        end

        if type(value) == 'boolean' then
            file:write(tostring(value));
        elseif type(value) == 'number' then
            file:write(value);
        elseif type(value) == 'string' then
            file:write(string.format('%q', value));
        elseif type(value) == 'table' then
            file:write('{\n');
            for _, v in ipairs(value) do
                indent(level + 1);
                serialize(file, v, level + 1);
                file:write(',\n');
            end
            for k, v in pairs(value) do
                if type(k) == 'string' and k ~= '__forge_hash' then
                    indent(level + 1);
                    file:write(('%s = '):format(k));
                    serialize(file, v, level + 1);
                    file:write(';\n');
                end
            end
            indent(level);
            file:write('}');
        end

        if level == 0 then
            file:write('\n');
        end
    end

    local local_settings = self.local_settings;
    if local_settings and local_settings.updated then
        local_settings.updated = nil;
        local cache_directory = self.cache_directory;
        local filename = cache_directory and root(('%s/local_settings.lua'):format(cache_directory)) or root('local_settings.lua');
        local file = io.open(filename, 'wb');
        assertf(file, 'Opening "%s" to write settings failed', filename);
        serialize(file, local_settings, 0);
        file:close();
    end
    mkdir(branch(forge.cache));
    save_binary();
end

forge.Toolset = require 'forge.Toolset';

return forge;
