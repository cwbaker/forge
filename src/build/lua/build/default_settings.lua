
local platforms_by_operating_system = {
    windows = { "android", "msvc", "mingw" };
    maxosx = { "llvmgcc" };
};

build.default_settings = {
    bin = root();
    lib = root();
    obj = root();
    data = root();
    root = root();

    user_settings_filename = home( "user_settings.lua" );

    local_settings_filename = root( "local_settings.lua" );

    include_directories = {};
    
    library_directories = {};

    platforms = platforms_by_operating_system [operating_system()];

    variants = {};

    settings_by_platform = {
        [""] = {
            architectures = {
                ""
            };
            variants = {
                "debug", "debug_dll", "release", "release_dll", "shipping", "shipping_dll"
            };
        };

        ["android"] = {
            architectures = { 
                "armv5", "armv7" 
            };
            variants = {
                "debug", "release", "shipping"
            };
        };

        ["llvmgcc"] = {
            architectures = {
                "x86_64"
            };
            variants = {
                "debug", "release", "shipping"
            };
        };

        ["mingw"] = {
            architectures = {
                "x86_64"
            };
            variants = {
                "debug", "debug_dll", "release", "release_dll", "shipping", "shipping_dll"
            };
        };

        ["msvc"] = {
            architectures = {
                "x86_64"
            };
            variants = {
                "debug", "debug_dll", "release", "release_dll", "shipping", "shipping_dll"
            };
        };
    };

    settings_by_variant = {
        [""] = {        
            library_type = "static";
        };

        ["debug"] = {
            compile_as_c = false;
            debug = true;
            exceptions = true;
            fast_floating_point = false;
            generate_map_file = true;
            incremental_linking = true;
            library_type = "static";
            link_time_code_generation = false;
            minimal_rebuild = true;
            optimization = false;
            pre_compiled_headers = true;
            preprocess = false;
            profiling = false;
            run_time_checks = true;
            runtime_library = "static_debug";
            run_time_type_info = true;
            sse2 = true;
            stack_size = 1048576;
            string_pooling = false;
            strip = false;
            subsystem = "CONSOLE";
            verbose_linking = false;
        };

        ["debug_dll"] = {
            compile_as_c = false;
            debug = true;
            exceptions = true;
            fast_floating_point = false;
            generate_map_file = true;
            incremental_linking = true;
            library_type = "dynamic";
            link_time_code_generation = false;
            minimal_rebuild = true;
            optimization = false;
            pre_compiled_headers = true;
            preprocess = false;
            profiling = false;
            run_time_checks = true;
            runtime_library = "dynamic_debug";
            run_time_type_info = true;
            sse2 = true;
            stack_size = 1048576;
            string_pooling = false;
            strip = false;
            subsystem = "CONSOLE";
            verbose_linking = false;
        };

        ["release"] = {
            compile_as_c = false;
            debug = true;
            exceptions = true;
            fast_floating_point = true;
            generate_map_file = true;
            incremental_linking = false;
            library_type = "static";
            link_time_code_generation = true;
            minimal_rebuild = false;
            optimization = true;
            pre_compiled_headers = true;
            preprocess = false;
            profiling = false;
            run_time_checks = false;
            runtime_library = "static";
            run_time_type_info = true;
            sse2 = true;
            stack_size = 1048576;        
            string_pooling = false;
            strip = false;
            subsystem = "CONSOLE";
            verbose_linking = false;
        };

        ["release_dll"] = {
            compile_as_c = false;
            debug = true;
            exceptions = true;
            fast_floating_point = true;
            generate_map_file = true;
            incremental_linking = false;
            library_type = "dynamic";
            link_time_code_generation = true;
            minimal_rebuild = false;
            optimization = true;
            pre_compiled_headers = true;
            preprocess = false;
            profiling = false;
            run_time_checks = false;
            runtime_library = "dynamic";
            run_time_type_info = true;
            sse2 = true;
            stack_size = 1048576;        
            string_pooling = false;
            strip = false;
            subsystem = "CONSOLE";
            verbose_linking = false;
        };
        
        ["shipping"] = {
            compile_as_c = false;
            debug = true;
            exceptions = true;
            fast_floating_point = true;
            generate_map_file = true;
            incremental_linking = false;
            library_type = "static";
            link_time_code_generation = true;
            minimal_rebuild = false;
            optimization = true;
            pre_compiled_headers = true;
            preprocess = false;
            profiling = true;
            run_time_checks = false;
            runtime_library = "static";
            run_time_type_info = true;
            sse2 = true;
            stack_size = 1048576;        
            string_pooling = false;
            strip = true;
            subsystem = "CONSOLE";
            verbose_linking = false;
        };
        
        ["shipping_dll"] = {
            compile_as_c = false;
            debug = true;
            exceptions = true;
            fast_floating_point = true;
            generate_map_file = true;
            incremental_linking = false;
            library_type = "dynamic";
            link_time_code_generation = true;
            minimal_rebuild = false;
            optimization = true;
            pre_compiled_headers = true;
            preprocess = false;
            profiling = true;
            run_time_checks = false;
            runtime_library = "dynamic";
            run_time_type_info = true;
            sse2 = true;
            stack_size = 1048576;
            string_pooling = false;
            strip = true;
            subsystem = "CONSOLE";
            verbose_linking = false;
        };    
    };    
}
