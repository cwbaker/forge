
# iOS Application

```
if build.platform_matches("ios.*") then
    local settings = build.push_settings {
        architectures = { "armv7", "armv7s" };
        sdkroot = "iphoneos";
        iphoneos_deployment_target = "6.1";
        targeted_device_family = "1,2";
        provisioning_profile = home( "sweet/sweet_software/rw_dev.mobileprovision" );
    };

    local app = ios.App "rw_ios" {
        entitlements = "Entitlements.lua";
        resource_rules = "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS8.2.sdk/ResourceRules.plist";
        xcode.Plist( "${data}/Info.plist", Generate("Info.plist", "Info.lua") );
        Copy( "${data}/Icon-57.png", "Icon-57.png" );
        Copy( "${data}/Icon-60.png", "Icon-60.png" );
        Copy( "${data}/Icon-72.png", "Icon-72.png" );
        Copy( "${data}/Icon-76.png", "Icon-76.png" );
        Copy( "${data}/Icon-120.png", "Icon-120.png" );
        Copy( "${data}/Icon-152.png", "Icon-152.png" );
        Copy( "${data}/Icon-1024.png", "Icon-1024.png" );
        Copy( "${data}/Default~iphone.png", "Default~iphone.png" );
        Copy( "${data}/Default@2x~iphone.png", "Default@2x~iphone.png" );
        Copy( "${data}/Default-568h@2x~iphone.png", "Default-568h@2x~iphone.png" );
        Copy( "${data}/Default~ipad.png", "Default~ipad.png" );
        Copy( "${data}/MPCloseButtonX.png", root("mopub-sdk-ios/Resources/MPCloseButtonX.png") );
        Copy( "${data}/MPCloseButtonX@2x.png", root("mopub-sdk-ios/Resources/MPCloseButtonX@2x.png") );
        Copy( "${data}/MRAID.bundle", root("mopub-sdk-ios/Resources/MRAID.bundle") );
        xcode.Xib {
            root( "mopub-sdk-ios/Internal/Common/MPAdBrowserController.xib" );
        };
    };

    local rw_ios = xcode.Lipo( "rw_ios" );
    app {
        rw_ios;
    };

    build.push_settings {
        bin = obj_directory( rw_ios );
    };
    for _, architecture in ipairs(settings.architectures) do
        rw_ios {
            Executable ("rw_ios", architecture) {
                framework_directories = {
                    root( "mopub-sdk-ios/AdNetworkSupport/Millennial/SDK" )
                };
                frameworks = {
                    "AdSupport",
                    "AudioToolbox",
                    "AVFoundation",
                    "CoreGraphics",
                    "CoreLocation",
                    "CoreTelephony",
                    "EventKit",
                    "EventKitUI",
                    "Foundation",
                    "UIKit",
                    "MediaPlayer",
                    "MessageUI",
                    "MobileCoreServices",
                    "OpenGLES",
                    "QuartzCore",
                    "StoreKit",
                    "SystemConfiguration",
                    "iAd",
                    "MillennialMedia"
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
                    "sweet/fx/fx_compiler/fx_compiler",
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
                    "mopub-sdk-ios/mopub-sdk-ios",
                    "lua/liblua",
                    "luasocket/socket",
                    "libzip/libzip",
                    "zlib/zlib"
                };
                ObjCxx (architecture) {
                    "AppDelegate.mm",
                    "IosGame.mm",
                    "OpenGLView.mm",
                    "rw_ios.mm"
                };
                Cxx (architecture) {
                    "Touches.cpp"
                };
            };
        };
    end
    build.pop_settings();
    build.pop_settings();
end
```