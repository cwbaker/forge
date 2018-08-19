
local AssetCatalog = forge:TargetPrototype( 'AssetCatalog' );

function AssetCatalog.create( forge, identifier, target_prototype, partial_info_plist )
	local assets = forge:File( ('%s/Assets.car'):format(identifier), AssetCatalog );
	if partial_info_plist then 
		assets:set_filename( forge:absolute(partial_info_plist), 2 );
	end
    assets:add_ordering_dependency( forge:Directory(assets:directory()) );
	return assets;
end

function AssetCatalog.build( forge, assets )
	local settings = assets.settings or forge:current_settings();
	local platform = settings.platform or 'ios';
    local sdkroot = settings.sdkroot or 'iphoneos';
    local xcrun = settings.xcrun or settings.ios.xcrun or settings.macos.xcrun;

    local args = {
    	'xcrun';
    	('--sdk %s'):format( sdkroot );
    	'actool';
		('--compile %s'):format( assets:ordering_dependency(1) );
		('--output-format %s'):format( assets.output_format or 'binary1' );
		('--compress-pngs');
	};

	local partial_info_plist = assets:filename(2);
	if partial_info_plist and partial_info_plist ~= '' then
		table.insert( args, ('--output-partial-info-plist %s'):format(partial_info_plist) );
	end

	table.insert( args, ('--platform %s'):format(sdkroot) );
	local minimum_deployment_target = settings[('%s_deployment_target'):format(sdkroot)];
	if minimum_deployment_target then 
		table.insert( args, ('--minimum-deployment-target %s'):format(minimum_deployment_target) );
	end

	local app_icon = assets.app_icon;
	if app_icon then
		table.insert( args, ('--app-icon %s'):format(app_icon) );
	end

	local launch_image = assets.launch_image;
	if launch_image then 
		table.insert( args, ('--launch-image %s'):format(launch_image) );
	end

	table.insert( args, assets:dependency():filename() );

    forge:system( xcrun, args );
end

xcode.AssetCatalog = AssetCatalog;
