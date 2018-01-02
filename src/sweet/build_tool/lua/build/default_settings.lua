
local platforms_by_operating_system = {
    windows = { "android", "windows" };
    macos = { "android", "ios", "ios_simulator", "macos" };
    linux = { "linux" };
};

local path_separator_by_operating_system = {
    windows = ";";
    macos = ":";
    linux = ":";
};

return {
    bin = build:root( ("../%s/bin"):format(variant) );
    lib = build:root( ("../%s/lib"):format(variant) );
    obj = build:root( ("../%s/obj"):format(variant) );
    gen = build:root( ("../%s/gen"):format(variant) );
    classes = build:root( ("../%s/classes"):format(variant) );
    cache = build:root( '.sweet_build' );

    platform = platform;

    path_separator = path_separator_by_operating_system [build:operating_system()];

    user_settings_filename = build:home( "user_settings.lua" );

    local_settings_filename = build:root( "local_settings.lua" );

    include_directories = {
        build:root();
    };
    
    library_directories = {
    };

    platforms = platforms_by_operating_system [build:operating_system()];

    variants = {};

    settings_by_platform = {
        [""] = {
            default_architecture = "";
            architectures = {
                ""
            };
        };

        ["android"] = {
            default_architecture = "armv7";
            architectures = { 
                "armv7" 
            };
            runtime_library = "gnustl_shared";
        };

        ["macos"] = {
            default_architecture = "x86_64";
            architectures = {
                "i386", "x86_64"
            };
            framework_directories = {
            };
        };

        ["ios"] = {
            default_architecture = "armv7";
            architectures = {
                "armv7", "arm64"
            };
            framework_directories = {
            };
            sdkroot = "iphoneos";
            iphoneos_deployment_target = "8.0";
            targeted_device_family = "1,2";
            provisioning_profile = build:home( "sweet/sweet_software/dev.mobileprovision" );
        };

        ["ios_simulator"] = {
            default_architecture = "x86_64";
            architectures = {
                "i386", "x86_64"
            };
            framework_directories = {
            };
            -- The "sdkroot" setting is replaced with "iphonesimulator" at 
            -- build time when building for the simulator.
            sdkroot = "iphoneos"; 
            iphoneos_deployment_target = "7.0";
            targeted_device_family = "1,2";
            provisioning_profile = build:home( "sweet/sweet_software/dev.mobileprovision" );
        };

        ["linux"] = {
            default_architecture = "x86_64";
            architectures = { 
                "x86_64" 
            };
        };

        ["windows"] = {
            default_architecture = "x86_64";
            architectures = {
                "x86_64"
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
            objc_modules = true;
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
            objc_modules = true;
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
            objc_modules = true;
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
    };    
}
