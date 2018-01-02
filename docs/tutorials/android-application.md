
# Android Application

```
if build.platform_matches("android") then
    local settings = build.push_settings {
        resources = {
            "res",
            root( "google-play-services/res" ),
            root( "facebook-android-sdk/facebook/res" )
        };
        jars = {
            "sweet/ads/ads.jar",
            "sweet/facebook/facebook.jar",
            "sweet/iap/iap.jar",
            "facebook-android-sdk/facebook/facebook-android-sdk.jar",
            "mopub-sdk-android/mopub-sdk-android.jar",
        };
        third_party_jars = {
            root( "facebook-android-sdk/libs/android-support-v4.jar" ),
            root( "mopub-sdk-android/libs/MMSDK.jar" ),
            root( "google-play-services/libs/google-play-services.jar" )
        };
    };

    local apk = android.Apk "rw_android" {
        Generate( "AndroidManifest.xml", "AndroidManifest.lua" );
        Generate( "proguard.cfg", "proguard.lua" );
        android.Dex "classes" {
            Java {
                sourcepath = "src";
                "nz/co/sweetsoftware/rw/AndroidIoPolicy.java",
                "nz/co/sweetsoftware/rw/RwActivity.java",
                "nz/co/sweetsoftware/rw/RwGame.java",
                "nz/co/sweetsoftware/rw/RwView.java",        
                android.BuildConfig "nz.co.sweetsoftware.rw";
                android.R { 
                    packages = {
                        "nz.co.sweetsoftware.rw",
                        "com.facebook.android",
                        "com.google.android.gms"
                    };
                    "res",
                    root( "google-play-services/res" ),
                    root( "facebook-android-sdk/facebook/res" )
                };
            };
        };
    };

    for _, architecture in ipairs(settings.architectures) do
        apk { 
            DynamicLibrary ("rw_android", architecture) {
                system_libraries = {
                    "OpenSLES",
                    "GLESv1_CM",
                    "gnustl_shared",
                    "android",
                    "log"
                };

                libraries = {
                    "sweet/red_wizard/controller/controller",
                    "sweet/red_wizard/model/model",
                    "sweet/red_wizard/view/view",
                    "sweet/resource/resource",
                    "sweet/audio/audio",
                    "sweet/ui2/ui2_lua/ui2_lua",
                    "sweet/ui2/ui2",
                    "sweet/math/math_lua/math_lua",
                    "sweet/facebook/facebook",
                    "sweet/fx/fx",
                    "sweet/fx/fx_virtual_machine/fx_virtual_machine",
                    "sweet/gfx/gfx",
                    "sweet/ai/ai_lua/ai_lua",
                    "sweet/ai/ai",
                    "sweet/ads/ads",
                    "sweet/iap/iap",
                    "sweet/lua/lua",
                    "sweet/persist/persist",
                    "sweet/path/path",
                    "sweet/io/io",
                    "sweet/thread/thread",
                    "sweet/rtti/rtti",
                    "sweet/error/error",
                    "sweet/pointer/pointer",
                    "sweet/assert/assert",
                    "lua/liblua",
                    "luasocket/socket",
                    "libzip/libzip",
                    "zlib/zlib"
                };

                Cxx (architecture) {
                    "jni/AndroidErrorPolicy.cpp",
                    "jni/AndroidGame.cpp",
                    "jni/AndroidIoPolicy.cpp",
                    "jni/rw_android.cpp"
                };
            };
        };
    end

    build.pop_settings();
end
```
