
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
    bin = forge:root( ("../%s/bin"):format(variant) );
    lib = forge:root( ("../%s/lib"):format(variant) );
    obj = forge:root( ("../%s/obj"):format(variant) );
    gen = forge:root( ("../%s/gen"):format(variant) );
    classes = forge:root( ("../%s/classes"):format(variant) );
    cache = forge:root( '.forge' );

    platform = platform;

    path_separator = path_separator_by_operating_system [forge:operating_system()];

    user_settings_filename = forge:home( "user_settings.lua" );

    local_settings_filename = forge:root( "local_settings.lua" );

    include_directories = {
        forge:root();
    };
    
    library_directories = {
    };

    platforms = platforms_by_operating_system [forge:operating_system()];

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
                'armv5', 'armv7' 
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
            provisioning_profile = forge:home( "sweet/sweet_software/dev.mobileprovision" );
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
            provisioning_profile = forge:home( "sweet/sweet_software/dev.mobileprovision" );
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
            assertions = true;
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
            assertions = true;
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
            assertions = false; 
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
