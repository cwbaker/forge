
local platforms_by_operating_system = {
    windows = { "android", "windows" };
    macosx = { "android", "ios", "ios_simulator", "macosx" };
};

local path_separator_by_operating_system = {
    windows = ";";
    macosx = ":";
};

return {
    bin = root( ("../%s_%s/bin"):format(platform, variant) );
    lib = root( ("../%s_%s/lib"):format(platform, variant) );
    obj = root( ("../%s_%s/obj"):format(platform, variant) );
    gen = root( ("../%s_%s/gen"):format(platform, variant) );
    classes = root( ("../%s_%s/classes"):format(platform, variant) );
    data = root( ("../%s_%s/data"):format(platform, variant) );
    root = root();

    path_separator = path_separator_by_operating_system [operating_system()];

    user_settings_filename = home( "user_settings.lua" );

    local_settings_filename = root( "local_settings.lua" );

    include_directories = {};
    
    library_directories = {};

    platforms = platforms_by_operating_system [operating_system()];

    variants = {};

    settings_by_platform = {
        [""] = {
            default_architecture = "";
            architectures = {
                ""
            };
            variants = {
                "debug", "release", "shipping"
            };
        };

        ["android"] = {
            default_architecture = "armv7";
            architectures = { 
                "armv7" 
            };
            variants = {
                "debug", "release", "shipping"
            };
            runtime_library = "gnustl_shared";
        };

        ["macosx"] = {
            default_architecture = "x86_64";
            architectures = {
                "i386", "x86_64"
            };
            variants = {
                "debug", "release", "shipping"
            };
            framework_directories = {
            };
        };

        ["ios"] = {
            default_architecture = "armv7";
            architectures = {
                "armv7", "arm64"
            };
            variants = {
                "debug", "release", "shipping"
            };
            framework_directories = {
            };
            sdkroot = "iphoneos";
            iphoneos_deployment_target = "7.0";
            targeted_device_family = "1,2";
            provisioning_profile = home( "sweet/sweet_software/dev.mobileprovision" );
        };

        ["ios_simulator"] = {
            default_architecture = "x86_64";
            architectures = {
                "i386", "x86_64"
            };
            variants = {
                "debug", "release", "shipping"
            };
            framework_directories = {
            };
            -- The "sdkroot" setting is replaced with "iphonesimulator" at 
            -- build time when building for the simulator.
            sdkroot = "iphoneos"; 
            iphoneos_deployment_target = "7.0";
            targeted_device_family = "1,2";
            provisioning_profile = home( "sweet/sweet_software/dev.mobileprovision" );
        };

        ["windows"] = {
            default_architecture = "i386";
            architectures = {
                "i386"
            };
            variants = {
                "debug", "release", "shipping"
            };
            third_party_libraries = {
                "advapi32",
                "gdi32",
                "kernel32",
                "user32",
                "ws2_32",
                "wsock32"
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
            debuggable = true;
            exceptions = true;
            fast_floating_point = false;
            generate_dsym_bundle = false;
            generate_map_file = true;
            incremental_linking = true;
            library_type = "static";
            link_time_code_generation = false;
            minimal_rebuild = true;
            objc_arc = true;
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
            warning_level = 3;
            warnings_as_errors = true;
        };

        ["debug_dll"] = {
            compile_as_c = false;
            debug = true;
            debuggable = true;
            exceptions = true;
            fast_floating_point = false;
            generate_dsym_bundle = false;
            generate_map_file = true;
            incremental_linking = true;
            library_type = "dynamic";
            link_time_code_generation = false;
            minimal_rebuild = true;
            objc_arc = true;
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
            warning_level = 3;
            warnings_as_errors = true;
        };

        ["release"] = {
            compile_as_c = false;
            debug = true;
            debuggable = true;
            exceptions = true;
            fast_floating_point = true;
            generate_dsym_bundle = false;
            generate_map_file = true;
            incremental_linking = false;
            library_type = "static";
            link_time_code_generation = true;
            minimal_rebuild = false;
            objc_arc = true;
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
            warning_level = 3;
            warnings_as_errors = true;
        };

        ["release_dll"] = {
            compile_as_c = false;
            debug = true;
            debuggable = true;
            exceptions = true;
            fast_floating_point = true;
            generate_dsym_bundle = false;
            generate_map_file = true;
            incremental_linking = false;
            library_type = "dynamic";
            link_time_code_generation = true;
            minimal_rebuild = false;
            objc_arc = true;
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
            warning_level = 3;
            warnings_as_errors = true;
        };
        
        ["shipping"] = {
            compile_as_c = false;
            debug = false;
            debuggable = false;
            exceptions = true;
            fast_floating_point = true;
            generate_dsym_bundle = true;
            generate_map_file = true;
            incremental_linking = false;
            library_type = "static";
            link_time_code_generation = true;
            minimal_rebuild = false;
            objc_arc = true;
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
            warning_level = 3;
            warnings_as_errors = true;
        };
        
        ["shipping_dll"] = {
            compile_as_c = false;
            debug = false;
            debuggable = false;
            exceptions = true;
            fast_floating_point = true;
            generate_dsym_bundle = true;
            generate_map_file = true;
            incremental_linking = false;
            library_type = "dynamic";
            link_time_code_generation = true;
            minimal_rebuild = false;
            objc_arc = true;
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
            warning_level = 3;
            warnings_as_errors = true;
        };
    };    
}
